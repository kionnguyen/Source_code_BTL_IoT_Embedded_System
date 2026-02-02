import os
import time
import threading
from datetime import datetime, timezone

import serial
from flask import Flask, jsonify, render_template, request
from supabase import create_client
from dotenv import load_dotenv
import httpx


# ================= LOAD ENV =================
load_dotenv()
SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_KEY")

if not SUPABASE_URL or not SUPABASE_KEY:
    print("SUPABASE_URL hoặc SUPABASE_KEY bị thiếu")
    raise SystemExit(1)

supabase = create_client(SUPABASE_URL, SUPABASE_KEY)
supabase.postgrest.session = httpx.Client(
    http2=False,
    timeout=httpx.Timeout(10.0, connect=10.0, read=10.0, write=10.0, pool=10.0),
)
# ================= SERIAL CONFIG =================
PORT = os.getenv("HC05_PORT", "COM7")
BAUD = int(os.getenv("HC05_BAUD", "9600"))

ser = None
ser_lock = threading.Lock()

def ensure_serial_open() -> bool:
    global ser
    if ser and getattr(ser, "is_open", False):
        return True
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.2, write_timeout=0.2)
        try:
            ser.reset_input_buffer()
        except Exception:
            pass
        print("[HC05] Connected on", PORT)
        return True
    except Exception as e:
        print("[HC05] Cannot open port:", e)
        ser = None
        return False

# ================= COMMAND ENCODING (1..4) =================
CODE_MAP = {
    ("fan", "ON"): 1,
    ("fan", "OFF"): 2,
    ("buzzer", "ON"): 3,
    ("buzzer", "OFF"): 4,
}

def build_command(device: str, state: str) -> tuple[int, bytes]:
    code = CODE_MAP.get((device, state))
    if code is None:
        raise ValueError("Invalid device/state")
    # gửi dạng ASCII + newline: b'1\n'
    return code, f"{code}\n".encode("utf-8")

def send_bytes_to_hc05(cmd: bytes) -> None:
    if not ensure_serial_open():
        raise RuntimeError(f"Serial not available on {PORT}")

    with ser_lock:
        ser.write(cmd)
        ser.flush()
        print(f"[HC05 TX] Sent: {cmd!r}")

        # ---- NEW: đọc phản hồi ACK để biết STM32 có nhận lệnh không ----
        try:
            ser.timeout = 0.3  # chờ tối đa 300ms
            ack = ser.readline()  # STM32 gửi "ACK x\r\n"
            if ack:
                print(f"[HC05 RX] {ack!r}  text='{ack.decode('utf-8', errors='replace').strip()}'")
            else:
                print("[HC05 RX] (no ACK within 300ms) -> STM32 may not receive/parse the command")
        finally:
            ser.timeout = 0.2  # trả lại timeout mặc định của bạn


# ================= SUPABASE HELPERS =================
def get_device_state() -> dict:
    res = supabase.table("device_state").select("*").eq("id", 1).limit(1).execute()
    if not res.data:
        state = {"fan": "OFF", "buzzer": "OFF", "updated_at": None}
        return state

    row = res.data[0]
    state = {
        "fan": row.get("fan", "OFF"),
        "buzzer": row.get("buzzer", "OFF"),
        "updated_at": row.get("updated_at"),
    }
    return state

def upsert_device_state(partial: dict) -> None:
    payload = {"id": 1, **partial, "updated_at": datetime.now(timezone.utc).isoformat()}
    supabase.table("device_state").upsert(payload).execute()

def update_sent_log(code: int, ok: bool) -> None:
    supabase.table("device_state").upsert({
        "id": 1,
        "last_sent_at": datetime.now(timezone.utc).isoformat(),
        "last_sent_cmd": code,
        "last_sent_ok": ok
    }).execute()

# ================= FLASK APP =================
app = Flask(__name__)

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/api/latest")
def latest():
    # gas_value lấy từ gas_data mới nhất
    gas_res = supabase.table("gas_data").select("*").order("created_at", desc=True).limit(1).execute()
    gas_value = int(gas_res.data[0]["gas_value"]) if gas_res.data else 0

    # fan/buzzer lấy từ device_state (đúng ngay sau khi bấm nút)
    st = get_device_state()

    return jsonify({
        "gas_value": gas_value,
        "fan": st["fan"],
        "buzzer": st["buzzer"],
    })

@app.post("/api/control")
def api_control():
    payload = request.get_json(silent=True) or {}
    device = payload.get("device")
    state  = payload.get("state")

    print("[/api/control] payload =", payload)

    if device not in ("fan", "buzzer") or state not in ("ON", "OFF"):
        return jsonify(ok=False, error="Invalid device/state"), 400

    try:
        update = {"id": 1, device: state}  # device = "fan" hoặc "buzzer"
        print("[/api/control] upsert =", update)

        resp = supabase.table("device_state").upsert(update).execute()

        # In ra để biết Supabase có trả data không
        print("[/api/control] supabase resp data =", getattr(resp, "data", None))

        # Nếu resp.data rỗng bất thường, vẫn coi là lỗi để bạn thấy ngay
        if not getattr(resp, "data", None):
            return jsonify(ok=False, error="Upsert returned no data (possible RLS/block)"), 500

        return jsonify(ok=True, device=device, state=state)

    except Exception as e:
        print("[/api/control] supabase error:", e)
        return jsonify(ok=False, error=str(e)), 500

# ================= THREAD: WATCH device_state -> SEND HC05 =================
def device_state_watcher():
    """
    Poll device_state; khi có thay đổi fan/buzzer, gửi mã 1..4 qua HC-05.
    """
    last_fan = None
    last_buzzer = None

    # lần đầu lấy state để không gửi bừa khi mới chạy
    try:
        st = get_device_state()
        last_fan = st["fan"]
        last_buzzer = st["buzzer"]
        print(f"[Watcher] init fan={last_fan}, buzzer={last_buzzer}")
    except Exception as e:
        print("[Watcher] init error:", e)

    while True:
        try:
            st = get_device_state()
            fan = st["fan"]
            buzzer = st["buzzer"]

            # nếu fan đổi -> gửi lệnh fan
            if fan in ("ON", "OFF") and fan != last_fan:
                code, cmd = build_command("fan", fan)
                # print(f"[Watcher] FAN changed {last_fan} -> {fan}, send {cmd!r}")
                try:
                    send_bytes_to_hc05(cmd)
                    update_sent_log(code, True)
                    last_fan = fan
                except Exception as e:
                    print("[Watcher] FAN send error:", e)
                    update_sent_log(code, False)

            # nếu buzzer đổi -> gửi lệnh buzzer
            if buzzer in ("ON", "OFF") and buzzer != last_buzzer:
                code, cmd = build_command("buzzer", buzzer)
                # print(f"[Watcher] BUZZER changed {last_buzzer} -> {buzzer}, send {cmd!r}")
                try:
                    send_bytes_to_hc05(cmd)
                    update_sent_log(code, True)
                    last_buzzer = buzzer
                except Exception as e:
                    print("[Watcher] BUZZER send error:", e)
                    update_sent_log(code, False)

        except Exception as e:
            print("[Watcher] loop error:", e)

        time.sleep(0.2)  # poll 200 ms

# ================= (OPTIONAL) BLUETOOTH READER: sensor -> gas_data =================
def bluetooth_reader():
    buffer = []
    BATCH_SIZE = 5

    while True:
        if not ensure_serial_open():
            time.sleep(2)
            continue

        try:
            with ser_lock:
                line = ser.readline().decode(errors="ignore").strip()
                print(f"[HC05 RX] line: {line!r}")
            if not line:
                continue

            # nếu echo code "1/2/3/4" thì bỏ
            if line in ("1", "2", "3", "4"):
                continue

            # format sensor: "gas fan buzzer" ví dụ: "350 1 0"
            parts = line.split()
            if len(parts) < 3:
                continue

            try:
                gas = int(parts[0])
                fan_raw = int(parts[1])
                buz_raw = int(parts[2])
            except:
                continue

            fan = "ON" if fan_raw == 1 else "OFF"
            buzzer = "ON" if buz_raw == 1 else "OFF"

            buffer.append({"gas_value": gas, "fan": fan, "buzzer": buzzer})

            if len(buffer) >= BATCH_SIZE:
                supabase.table("gas_data").insert(buffer).execute()
                buffer.clear()

        except Exception as e:
            print("[HC05 RX ERROR]", e)
            try:
                if ser:
                    ser.close()
            except:
                pass
            time.sleep(2)

# ================= MAIN =================
if __name__ == "__main__":
    # Thread gửi lệnh theo device_state
    threading.Thread(target=device_state_watcher, daemon=True).start()

    # Nếu bạn vẫn cần đọc sensor từ cùng cổng HC-05, bật thread này
    threading.Thread(target=bluetooth_reader, daemon=True).start()

    print("Flask server running...")
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=False)
