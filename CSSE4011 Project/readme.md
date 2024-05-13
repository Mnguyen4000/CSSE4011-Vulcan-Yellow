# Installation Instructions
Make sure current python version is 3.12

pip install -r packages.txt

Create two seperate terminals.

Terminal 1:
python ./web.py

Terminal 2:
python ./main.py

On a internet browser, go to url: localhost:5000 to view dashboard.

If weather data is not updating after refresh of page or you cannot view the dashboard, check the output of terminal 1 and connect to the address displayed there,
then change the flask url in main.py to this address.

