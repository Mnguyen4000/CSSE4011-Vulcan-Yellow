from flask import Flask, render_template, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime

app = Flask(__name__, template_folder='templates')
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///weatherdata.db'
db = SQLAlchemy(app)


# Define a model
class SensorData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    sensor_id = db.Column(db.String(10))
    timestamp = db.Column(db.DateTime)
    temperature = db.Column(db.Float)
    humidity = db.Column(db.Float)
    tvoc = db.Column(db.Integer)


# Create database tables
with app.app_context():
    db.create_all()

# Function to group sensor data by hour
def group_data_by_hour(sensor_data):
    grouped_data = {}
    for data_point in sensor_data:
        hour_key = data_point.timestamp.strftime('%Y-%m-%d %H:00:00')
        if hour_key not in grouped_data:
            grouped_data[hour_key] = []
        grouped_data[hour_key].append(data_point)
    # Sort data within each hour group
    for hour_key, data_points in grouped_data.items():
        data_points.sort(key=lambda x: x.timestamp)
    return grouped_data




# Endpoint to receive and store sensor data
@app.route('/timestamps', methods=['POST'])
def receive_timestamps():
    data = request.get_json()
    for sensor_id, sensor_data in data.items():
        for reading in sensor_data:
            # Extract data from the JSON
            timestamp = datetime.fromisoformat(reading['timestamp'])
            temperature = reading['temperature']
            humidity = reading['humidity']
            tvoc = reading['tvoc']

            # Create a new SensorData object and add it to the database session
            sensor_reading = SensorData(sensor_id=sensor_id,
                                        timestamp=timestamp,
                                        temperature=temperature,
                                        humidity=humidity,
                                        tvoc=tvoc)
            db.session.add(sensor_reading)

    # Commit the changes to the database
    db.session.commit()

    return jsonify({'message': 'Timestamps received and stored successfully'})


# Endpoint to send timestamp data to frontend
@app.route('/get_timestamps', methods=['GET'])
def get_timestamps():
    # Fetch data from the database and convert it to JSON
    sensor_data = SensorData.query.all()
    data = {}
    for reading in sensor_data:
        if reading.sensor_id not in data:
            data[reading.sensor_id] = []
        data[reading.sensor_id].append({
            'timestamp': reading.timestamp.isoformat(),
            'temperature': reading.temperature,
            'humidity': reading.humidity,
            'tvoc': reading.tvoc
        })
    return jsonify(data)


# Endpoint to view the timestamp data
@app.route('/view_timestamps')
def view_timestamps():
    # Fetch data from the database and pass it to the template
    sensor_data = SensorData.query.all()
    timestamps = {}
    for reading in sensor_data:
        if reading.sensor_id not in timestamps:
            timestamps[reading.sensor_id] = []
        timestamps[reading.sensor_id].append({
            'timestamp': reading.timestamp.isoformat(),
            'temperature': reading.temperature,
            'humidity': reading.humidity,
            'tvoc': reading.tvoc
        })
    return render_template('view_timestamps.html', timestamps=timestamps)

def group_and_average_sensor_data(sensor1_data, sensor2_data, sensor3_data):
    # Combine all sensor data into one list
    all_sensor_data = sensor1_data + sensor2_data + sensor3_data

    # Group data by timestamp (minute-level precision)
    grouped_data = {}
    for data_point in all_sensor_data:
        timestamp_key = data_point.timestamp.strftime('%Y-%m-%d %H:%M:00')  # Minute-level precision
        if timestamp_key not in grouped_data:
            grouped_data[timestamp_key] = {'temperature': [], 'humidity': [], 'tvoc': []}
        grouped_data[timestamp_key]['temperature'].append(data_point.temperature)
        grouped_data[timestamp_key]['humidity'].append(data_point.humidity)
        grouped_data[timestamp_key]['tvoc'].append(data_point.tvoc)

    # Calculate averages
    for timestamp_key, data in grouped_data.items():
        temperature_avg = sum(data['temperature']) / len(data['temperature'])
        humidity_avg = sum(data['humidity']) / len(data['humidity'])
        tvoc_avg = sum(data['tvoc']) / len(data['tvoc'])
        grouped_data[timestamp_key] = {'temperature': temperature_avg, 'humidity': humidity_avg, 'tvoc': tvoc_avg}

    return grouped_data

@app.route('/')
def dashboard():
    sensor1_data = SensorData.query.filter_by(sensor_id='sensor1').order_by(SensorData.timestamp).all()
    sensor2_data = SensorData.query.filter_by(sensor_id='sensor2').order_by(SensorData.timestamp).all()
    sensor3_data = SensorData.query.filter_by(sensor_id='sensor3').order_by(SensorData.timestamp).all()
    grouped_data = group_and_average_sensor_data(sensor1_data, sensor2_data, sensor3_data)
    sorted_keys = sorted(grouped_data.keys())
    min_temperature = min(data['temperature'] for data in grouped_data.values())
    max_temperature = max(data['temperature'] for data in grouped_data.values())
    min_humidity = min(data['humidity'] for data in grouped_data.values())
    max_humidity = max(data['humidity'] for data in grouped_data.values())
    min_tvoc = min(data['tvoc'] for data in grouped_data.values())
    max_tvoc = max(data['tvoc'] for data in grouped_data.values())

    return render_template('dashboard.html',
                           grouped_data=grouped_data,
                           sorted_keys=sorted_keys,
                           min_temperature=min_temperature,
                           max_temperature=max_temperature,
                           min_humidity=min_humidity,
                           max_humidity=max_humidity,
                           min_tvoc=min_tvoc,
                           max_tvoc=max_tvoc)

# Endpoint for sensor 1 page
@app.route('/sensor1')
def sensor1():
    sensor_data = SensorData.query.filter_by(sensor_id='sensor1').order_by(SensorData.timestamp).all()
    grouped_data = group_data_by_hour(sensor_data)
    return render_template('sensor1.html', grouped_data=grouped_data)



# Endpoint for sensor 2 page
@app.route('/sensor2')
def sensor2():
    sensor_data = SensorData.query.filter_by(sensor_id='sensor2').order_by(SensorData.timestamp).all()
    grouped_data = group_data_by_hour(sensor_data)
    return render_template('sensor2.html', grouped_data=grouped_data)

# Endpoint for sensor 3 page
@app.route('/sensor3')
def sensor3():
    sensor_data = SensorData.query.filter_by(sensor_id='sensor3').order_by(SensorData.timestamp).all()
    grouped_data = group_data_by_hour(sensor_data)
    return render_template('sensor3.html', grouped_data=grouped_data)



# Run the Flask app
if __name__ == '__main__':
    app.run(debug=True)
