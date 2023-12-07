import serial
import mysql.connector
import time
import threading
from flask import Flask, request

# ---------------------- 전역 변수 ----------------------
current_mode = "Unknown"
day_temperature = 0
night_temperature = 0

# ---------------------- Flask 설정 ----------------------
app = Flask(__name__)

@app.route("/send_data", methods=['POST'])
def send_data():
    global day_temperature, night_temperature
    received_data = request.json
    day_temperature = received_data.get("day_temperature", 0)
    night_temperature = received_data.get("night_temperature", 0)

    send_temperatures_to_arduino(day_temperature, night_temperature)
    return "Data received successfully"

# ---------------------- Database Configuration ----------------------

DB_CONFIG = {
    'host': 'project-db-stu3.smhrd.com',
    'user': 'Insa4_IOTA_hacksim_1',
    'password': 'aishcool1',
    'database': 'Insa4_IOTA_hacksim_1',
    'port': 3307
}

MAX_RETRIES = 3
RETRY_DELAY = 5

# ---------------------- Database Functions ----------------------

def get_db_connection():
    try:
        connection = mysql.connector.connect(**DB_CONFIG)
        return connection
    except mysql.connector.Error as err:
        print("Failed to connect to the database:", err)
        return None

def insert_into_db(sensor_id, temperature, humidity):
    connection = get_db_connection()
    if not connection:
        print("Cannot establish a connection to the database.")
        return

    cursor = connection.cursor()
    retries = 0
    while retries < MAX_RETRIES:
        try:
            temp_sql = "INSERT INTO temp (Sensor_ID, Temp_Value) VALUES (%s, %s)"
            cursor.execute(temp_sql, (sensor_id, float(temperature)))
            
            humidity_sql = "INSERT INTO rehum (Sensor_ID, REhum_Value) VALUES (%s, %s)"
            cursor.execute(humidity_sql, (sensor_id, float(humidity)))

            connection.commit()
            break

        except mysql.connector.Error as err:
            retries += 1
            print(f"Attempt {retries} failed with error {err}. Retrying in {RETRY_DELAY} seconds...")
            time.sleep(RETRY_DELAY)

    cursor.close()
    connection.close()

    if retries == MAX_RETRIES:
        print("Failed to insert into database after maximum retries.")


# ---------------------- Arduino 함수 ----------------------
def get_arduino_connection():
    try:
        ser = serial.Serial('/dev/ttyACM0', 9600)
        time.sleep(2)  # Arduino가 재설정되는 데 시간을 줍니다
        return ser
    except serial.SerialException as e:
        print("Arduino에 연결하는 데 실패했습니다:", e)
        return None

def get_current_mode():
    hour = time.localtime().tm_hour
    return 'True' if 8 <= hour < 20 else 'False'

def update_mode_on_arduino(ser):
    """Regularly checks the time and sends the mode to the Arduino."""
    global current_mode
    while True:
        new_mode = get_current_mode()
        if new_mode != current_mode:
            ser.write(f'RT{new_mode}\n'.encode())
            current_mode = new_mode
        time.sleep(60)  # Update every 10 minutes


def listen_to_arduino(ser):
    global current_mode
    
    while True:
        if ser.inWaiting():
            data = ser.readline().decode('utf-8').strip()
            
            if "TRSensor" in data:
                values = data.split(',')
                for i in range(4):
                    sensor_id = values[3 * i + 1]
                    temperature = values[3 * i + 2]
                    humidity = values[3 * i + 3]
                    insert_into_db(sensor_id, temperature, humidity)
                    print(sensor_id, temperature, humidity)

            elif "Mode Changed Successfully" in data:
                current_mode = get_current_mode()
                print(f"Arduino Mode Updated: {current_mode}")

            elif "ACK:" in data:
                print(f"Acknowledgment from Arduino: {data}")
            
        time.sleep(5)

def send_temperatures_to_arduino(day_temp, night_temp):
    with get_arduino_connection() as ser:
        temp_msg = f'BT,{day_temp},{night_temp}\n'
        ser.write(temp_msg.encode())
        time.sleep(3)
        if ser.inWaiting():
            ack_msg = ser.readline().decode('utf-8').strip()
            print(f"Arduino에서 받은 데이터: {ack_msg}")

def run_flask_server():
    app.run(host='0.0.0.0', port=5000)

# ---------------------- 메인 실행 ----------------------
if __name__ == "__main__":
    with get_arduino_connection() as ser:
        if not ser:
            print("Arduino에 연결할 수 없습니다.")
        else:
            mode_update_thread = threading.Thread(target=update_mode_on_arduino, args=(ser,))
            listen_thread = threading.Thread(target=listen_to_arduino, args=(ser,))
            flask_thread = threading.Thread(target=run_flask_server)

            mode_update_thread.start()
            listen_thread.start()
            flask_thread.start()

            mode_update_thread.join()
            listen_thread.join()
