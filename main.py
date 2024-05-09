import json
import serial
from datetime import datetime, timedelta
import time

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
        #data = json.loads(dataString)
        print(ser.readline().decode("ascii"))
        data = json.loads(ser.readline().decode("ascii"))
        return data[str(self.sensor_id)]


def collect_data_for_minute():
    timestamps = {'sensor1': None, 'sensor2': None, 'sensor3': None}
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

if __name__ == '__main__':
    while (1):
        timestamps = collect_data_for_minute()
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

