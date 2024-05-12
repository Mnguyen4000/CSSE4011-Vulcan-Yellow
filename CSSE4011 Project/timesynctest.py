from datetime import datetime, timedelta
import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt

# Step 1: Collect Timestamps
timestamps = {
    'sensor1': [datetime(2024, 5, 8, 13, 30, 3, 100000),
                datetime(2024, 5, 8, 13, 30, 3, 200000),
                datetime(2024, 5, 8, 13, 30, 3, 300000),
                datetime(2024, 5, 8, 13, 30, 3, 400000),
                datetime(2024, 5, 8, 13, 30, 3, 500000)],
    'sensor2': [datetime(2024, 5, 8, 13, 30, 3, 150000),
                datetime(2024, 5, 8, 13, 30, 3, 250000),
                datetime(2024, 5, 8, 13, 30, 3, 350000),
                datetime(2024, 5, 8, 13, 30, 3, 450000),
                datetime(2024, 5, 8, 13, 30, 3, 550000)],
    'sensor3': [datetime(2024, 5, 8, 13, 30, 3, 175000),
                datetime(2024, 5, 8, 13, 30, 3, 275000),
                datetime(2024, 5, 8, 13, 30, 3, 375000),
                datetime(2024, 5, 8, 13, 30, 3, 475000),
                datetime(2024, 5, 8, 13, 30, 3, 575000)]
}



# Step 2: Compute Time Differences
time_diffs = []
sensor_pairs = [('sensor1', 'sensor2'), ('sensor1', 'sensor3'), ('sensor2', 'sensor3')]

for pair in sensor_pairs:
    diffs = [(t2 - t1).total_seconds() * 1000 for t1, t2 in zip(timestamps[pair[0]], timestamps[pair[1]])]
    time_diffs.append(diffs)

print(time_diffs)

# Step 3: Create Feature Matrix
X = np.array(time_diffs).T
print(X)

# Step 4: Create Target Vector
y = np.arange(len(X))
print(y)

# Step 5: Fit Multiple Linear Regression Model
reg = LinearRegression().fit(X, y)

# Step 6: Continuous Correction
def correct_timestamp(timestamp, sensor_id):
    drift_correction = reg.intercept_ + sum([coeff * (timestamp - timestamps[s_id][0]).total_seconds() * 1000 for s_id, coeff in zip(timestamps.keys(), reg.coef_)])  # Convert to milliseconds
    return timestamp - timedelta(milliseconds=drift_correction)  # Convert to timedelta in milliseconds

# Example correction and print
corrected_timestamps = []
for sensor_id in timestamps:
    for time in timestamps[sensor_id]:
        original = time
        corrected = correct_timestamp(time, sensor_id)
        print(f"Original: {original}, Corrected: {corrected}")
        corrected_timestamps.append(corrected)

# Plot original data points and regression line
plt.figure(figsize=(10, 6))

# Plot original data points
for sensor_id, times in timestamps.items():
    plt.scatter(times, np.arange(len(times)), label=f'Original Sensor {sensor_id}', alpha=0.5)

# Plot regression line
plt.plot(corrected_timestamps, np.arange(len(corrected_timestamps)), color='green', linewidth=2, label='Regression Line')

plt.xlabel('Timestamps')
plt.ylabel('Sample Index')
plt.title('Original Timestamps and Regression Line')
plt.legend()
plt.grid(True)
plt.show()
