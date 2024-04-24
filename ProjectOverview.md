# Project and Scenario Description
The project requires the users to construct and integrate a network of bluetooth devices to measure the air quality and to act as a weather station. This would be done through the use of 3 nodes, each having the capability to measure the air quality. The task will be using 2 Thingy52 and 1 SEN54 to detect the air quality. It will involve integrating the systems together to get valueable data on the current weather conditions and will be displayed on a web dashboard viewer for ease of access.

# at least 5 Key Performance Indicators- how is the ’success’ of the project measured?


# System Overview (Hardware Architecture- block diagram of the system, Top-level flowchart of software implementation (mote and PC).


# Sensor Integration- What sensors are used? What type of data is required? How are the sensors integrated?
The pair of thingy52 will use their air quality sensors (CCS811) to detect and then relay the information onto the base node (nrf52840dk_nrf52840) via bluetooth advertising.
The SEN54 will be equiped with its own VOC sensors to transmit the data to the base node through a uart connection.

# Wireless Network Communication or IoT protocols/Web dashboards- e.g. What is the network topology or IoT protocols used? What protocols are used and how?, What sort of data rate is required? You must also include a message protocol diagram.


# Algorithms schemes used- e.g. Machine learning approaches


# DIKW Pyramid abstraction. Provide a scenario in which your system can operate. Consider what DIKW Pyramid layers that your system can contribute to.

## Data:
The data that will be extract from the project will be the immediate air quality of the vicinity. It will measure the CO2 levels and volatile organic compounds.

## Information:
Volatile organic compounds are man-made chemicals such as Formaldehyde, which forms in the atmosphere by heating plastics, or Methylen chloride which is present in paint removers, and other flame retardant chemicals. By detecting the atmosphere's VOC levels, we will be able to evaluate if the atmosphere is at a safe level.

Carbon Dioxide is molecule which traps heats in our atmosphere, and is therefore an important green house gas but can also aid global warming. The effects of Carbon Dioxide poisoning are also deadly if exposed in close proximity to it for too long. This would include having headaches, drowsiness and eventual death if the subject does not remove themself from the vicinity.

## Knowledge:



## Wisdom:

By acquiring the information of the air quality, we can determine if atmosphere is at a safe level soley through technological means instead of sampling the air and testing it through chemicals. It will allow us to get an almost instantanous value in which we can access at a safe remote distance. For example this could help in environments such as mines, where they could keep a constant reading to let the workers know if the levels reach a dangerous level. Another example could be in more pollutant cities where they take data periodically and compare it with previous data to keep a constant track of the pollution levels.
