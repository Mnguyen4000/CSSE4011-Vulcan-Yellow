import json
import serial
from datetime import datetime

dataString = '''
{
    "1": [{
         "Timestamp": "13:30:03",
         "Temperature": "25",
         "Humidity": "41",
         "TVOC": "232"
         }],
    "2": [{
         "Timestamp": "13:30:04",
         "Temperature": "23",
         "Humidity": "50",
         "TVOC": "259"
         }],
    "3": [{
         "Timestamp": "13:30:05",
         "Temperature": "24",
         "Humidity": "40",
         "TVOC": "230"
         }]
}
'''

# ser = serial.Serial("COM3", 115200)
# ser.timeout = 1

class WeatherSensor:
    def __init__(self, sensor_id):
        self.sensor_id = sensor_id

    def collect_data(self):
        data = json.loads(dataString)
        return data[str(self.sensor_id)]


def synchronize_data(sensor_data):
    synced_data = {}
    reference_time = None
    current_date = datetime.now().date()

    for sensor_id, data_list in sensor_data.items():
        for data in data_list:
            timestamp_str = f"{current_date} {data['Timestamp']}"
            timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S')

            if reference_time is None:
                reference_time = timestamp
                time_offset = 0
            else:
                time_offset = (timestamp - reference_time).total_seconds()

            iso_timestamp = timestamp.isoformat()
            if iso_timestamp not in synced_data:
                synced_data[iso_timestamp] = []

            synced_data[iso_timestamp].append({
                "sensor_id": sensor_id,
                "time_offset": time_offset,
                "data": data
            })

    return synced_data

if __name__ == '__main__':
    sensor1 = WeatherSensor(1)
    sensor2 = WeatherSensor(2)
    sensor3 = WeatherSensor(3)

    sensor_data = {
        1: sensor1.collect_data(),
        2: sensor2.collect_data(),
        3: sensor3.collect_data()
    }

    synced_data = synchronize_data(sensor_data)

    print(json.dumps(synced_data, indent=2))


# print(str(datetime.now()))
# for beacon in data['1']:
#     print("Temp: " + beacon['Temperature'])
#     print("Humidity: " + beacon['Humidity'])
#     print("TVOC: " + beacon['TVOC'])

