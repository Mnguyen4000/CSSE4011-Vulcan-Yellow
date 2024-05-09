import json
import serial
from datetime import datetime, timedelta
import time
import numpy as np
from sklearn.linear_model import LinearRegression

dataString = '''
{
    "1": [{
         "Timestamp": "100000",
         "Temperature": "25",
         "Humidity": "41",
         "TVOC": "232"
         }],
    "2": [{
         "Timestamp": "100000",
         "Temperature": "23",
         "Humidity": "50",
         "TVOC": "259"
         }],
    "3": [{
         "Timestamp": "100000",
         "Temperature": "24",
         "Humidity": "40",
         "TVOC": "230"
         }]
}
'''

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
    def __init__(self, sensor_id):
        self.sensor_id = sensor_id

    def collect_data(self):
        # data = json.loads(dataString)
        print(ser.readline().decode("ascii"))
        data = json.loads(ser.readline().decode("ascii"))
        return data[str(self.sensor_id)]


def collect_data_for_minute():
    timestamps = {'sensor1': None, 'sensor2': None, 'sensor3': None}
    # change timedelta to extend or reduce collation interval
    end_time = datetime.now() + timedelta(seconds=1)
    while datetime.now() < end_time:
        sensor1 = WeatherSensor(1)
        sensor2 = WeatherSensor(2)
        sensor3 = WeatherSensor(3)
        sensor_data = {
            1: sensor1.collect_data(),
            2: sensor2.collect_data(),
            3: sensor3.collect_data()
        }
        current_date = datetime.now().date()
        for sensor_id, data_list in sensor_data.items():
            for data in data_list:
                current_time_str = datetime.now().strftime("%H:%M:%S")
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
                    #delay for testing
                time.sleep(0.1)
    return timestamps


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
            diffs.append((sensor2_timestamps[i] - sensor1_timestamps[i]).total_seconds() * 1000)
        time_diffs.append(diffs)

    print("Time differences between sensors: ")
    print(time_diffs)

    # Fit linear regression model
    X = np.array(time_diffs).T
    y = np.arange(len(X))
    reg = LinearRegression().fit(X, y)

    # Correct timestamps using linear regression
    def correct_timestamp(timestamp, sensor_id):
        drift_correction = reg.intercept_ + sum(
            [coeff * (timestamp - timestamps[s_id].timestamp).total_seconds() * 1000 for s_id, coeff in
             zip(timestamps.keys(), reg.coef_)])  # Convert to milliseconds
        return timestamp - timedelta(milliseconds=drift_correction)  # Convert to timedelta in milliseconds

    # Iterate through original timestamps and correct them
    for sensor_id, reading in timestamps.items():
        current = reading
        while current:
            original = current.timestamp
            corrected = correct_timestamp(original, sensor_id)
            print(f"{sensor_id}: Original: {original}, Corrected: {corrected}")
            current.timestamp = corrected  # Reassign the corrected timestamp to the original list
            current = current.next

    return timestamps


if __name__ == '__main__':
    # while (1):
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
