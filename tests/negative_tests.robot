# Author: happla


*** Settings ***
Library           ESP32Serial.py
Library           ESP32GPIO.py

Suite Setup       Connect To Device    115200
Suite Teardown    Close Connection


*** Test Cases ***
Six Pulses Should Not Pass Five Pulse Expectation
    ${count}=    Measure LED Pulses    1.0    6
    Should Be Equal As Integers    ${count}    5
