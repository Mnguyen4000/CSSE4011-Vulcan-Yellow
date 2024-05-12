import json
import serial
from datetime import datetime, timedelta
import time
import numpy as np
from sklearn.linear_model import LinearRegression
import requests

# True for test string, false for serial comm
debug = True

FLASK_URL = 'http://localhost:5000'

data_strings = [
    '''
    {
        "1": [{
             "Timestamp": "100000",
             "Temperature": "32",
             "Humidity": "66",
             "TVOC": "1003"
             }],
        "2": [{
             "Timestamp": "110000",
             "Temperature": "37",
             "Humidity": "65",
             "TVOC": "1005"
             }],
        "3": [{
             "Timestamp": "120000",
             "Temperature": "38",
             "Humidity": "64",
             "TVOC": "1009"
             }]
    }
    ''',
    '''
    {
        "1": [{
             "Timestamp": "203000",
             "Temperature": "34",
             "Humidity": "77",
             "TVOC": "1212"
             }],
        "2": [{
             "Timestamp": "215000",
             "Temperature": "38",
             "Humidity": "76",
             "TVOC": "1122"
             }],
        "3": [{
             "Timestamp": "228000",
             "Temperature": "35",
             "Humidity": "77",
             "TVOC": "1002"
             }]
    }
    ''',
    '''
    {
        "1": [{
             "Timestamp": "307000",
             "Temperature": "32",
             "Humidity": "65",
             "TVOC": "1111"
             }],
        "2": [{
             "Timestamp": "320000",
             "Temperature": "38",
             "Humidity": "65",
             "TVOC": "1232"
             }],
        "3": [{
             "Timestamp": "332000",
             "Temperature": "39",
             "Humidity": "67",
             "TVOC": "1232"
             }]
    }
    '''
]

if debug == False:
    ser = serial.Serial("COM3", 115200)
    ser.timeout = 1
class SensorReading:
    def __init__(self, timestamp, temperature, humidity, tvoc):
        self.timestamp = timestamp
        self.temperature = temperature
        self.humidity = humidity
        self.tvoc = tvoc
        self.next = None

class WeatherSensor:
    def __init__(self, sensor_id, data_string):
        self.sensor_id = sensor_id
        self.data_string = data_string

    def collect_data(self):
        data = json.loads(self.data_string)
        return data[str(self.sensor_id)]

def collect_data_for_minute():
    timestamps = {'sensor1': None, 'sensor2': None, 'sensor3': None}
    current_date = datetime.now().date()  # Retrieve the current date once
    start_time = datetime.now()  # Record the start time
    while True:
        if debug == False:
            data = ser.readline().decode("ascii")
        else:
            data = data_strings[0]

        current_time = datetime.now()  # Retrieve the current time for each iteration
        if (current_time - start_time).seconds >= 10:  # Check if a minute has passed
            break
        sensor1 = WeatherSensor(1, data)
        sensor2 = WeatherSensor(2, data)
        sensor3 = WeatherSensor(3, data)
        sensor_data = {
            1: sensor1.collect_data(),
            2: sensor2.collect_data(),
            3: sensor3.collect_data()
        }
        for sensor_id, data_list in sensor_data.items():
            for data in data_list:
                current_time_str = current_time.strftime("%H:%M:%S")  # Use the current time
                current_time_str += ("." + data['Timestamp'])
                timestamp_str = f"{current_date} {current_time_str}"
                timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S.%f')
                reading = SensorReading(timestamp, float(data['Temperature']), float(data['Humidity']), float(data['TVOC']))
                if timestamps[f'sensor{sensor_id}'] is None:
                    timestamps[f'sensor{sensor_id}'] = reading
                else:
                    current = timestamps[f'sensor{sensor_id}']
                    while current.next:
                        current = current.next
                    current.next = reading
                time.sleep(0.1)


    return timestamps

def send_timestamps_to_flask(timestamps):
    url = f"{FLASK_URL}/timestamps"
    headers = {'Content-Type': 'application/json'}

    # Convert sensor data to the expected format
    data = {}
    for sensor_id, readings in timestamps.items():
        sensor_readings = []
        current = readings
        while current:
            sensor_readings.append({
                'timestamp': str(current.timestamp),
                'temperature': current.temperature,
                'humidity': current.humidity,
                'tvoc': current.tvoc
            })
            current = current.next
        data[sensor_id] = sensor_readings

    # Send the data to the Flask server
    response = requests.post(url, data=json.dumps(data), headers=headers)
    if response.status_code == 200:
        print("Data sent to Flask successfully.")
    else:
        print("Failed to send data to Flask.")


def synchronise_timestamps(timestamps):
    time_diffs = []
    sensor_pairs = [('sensor1', 'sensor2'), ('sensor1', 'sensor3'), ('sensor2', 'sensor3')]

    # Calculate time differences between sensors
    for pair in sensor_pairs:
        diffs = []
        sensor1_timestamps = []
        sensor2_timestamps = []
        current1 = timestamps[pair[0]]
        current2 = timestamps[pair[1]]
        while current1 and current2:
            sensor1_timestamps.append(current1.timestamp)
            sensor2_timestamps.append(current2.timestamp)
            current1 = current1.next
            current2 = current2.next
        min_length = min(len(sensor1_timestamps), len(sensor2_timestamps))
        for i in range(min_length):
            diffs.append((sensor2_timestamps[i] - sensor1_timestamps[i]).total_seconds())
        time_diffs.append(diffs)

    print("Time differences between sensors: ")
    print("sensor1 - sensor2")
    print(time_diffs[0])
    print("sensor1 - sensor3")
    print(time_diffs[1])
    print("sensor2 - sensor3")
    print(time_diffs[2])
    print("\n")


    # Fit linear regression model
    X = np.array(time_diffs).T
    y = np.arange(len(X))
    reg = LinearRegression().fit(X, y)

    # Correct timestamps using linear regression
    def correct_timestamp(timestamp, sensor_id):
        drift_correction = reg.intercept_ + sum(
            [coeff * (timestamp - timestamps[s_id].timestamp).total_seconds() for s_id, coeff in
             zip(timestamps.keys(), reg.coef_)])  # Convert to milliseconds
        return timestamp - timedelta(microseconds=drift_correction)  # Convert to timedelta in milliseconds

    # Iterate through original timestamps and correct them
    for sensor_id, reading in timestamps.items():
        current = reading
        while current:
            original = current.timestamp
            corrected = correct_timestamp(original, sensor_id)
            print(f"{sensor_id}: Original: {original}, Corrected: {corrected}")
            current.timestamp = corrected  # Reassign the corrected timestamp to the original list
            current = current.next

    send_timestamps_to_flask(timestamps)
    return timestamps


if __name__ == '__main__':
    # while (1):
    # for data_string in data_strings:
        print("New Readings: \n")
        timestamps = collect_data_for_minute()
        for sensor_id, reading in timestamps.items():
            print(f"{sensor_id}:")
            current = reading
            while current:
                print(
                    f"Timestamp: {current.timestamp}, Temperature: {current.temperature}, Humidity: {current.humidity}, TVOC: {current.tvoc}")
                current = current.next

        print("\n")

        synchronise_timestamps(timestamps)
        print("\n")

        print("Corrected timestamps: \n")

        for sensor_id, reading in timestamps.items():
            print(f"{sensor_id}:")
            current = reading
            while current:
                print(
                    f"Timestamp: {current.timestamp}, Temperature: {current.temperature}, Humidity: {current.humidity}, TVOC: {current.tvoc}")
                current = current.next



# print(str(datetime.now()))
# for beacon in data['1']:
#     print("Temp: " + beacon['Temperature'])
#     print("Humidity: " + beacon['Humidity'])
#     print("TVOC: " + beacon['TVOC'])
