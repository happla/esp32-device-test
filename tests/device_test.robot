# Author: happla

*** Settings ***
Library           ESP32Serial.py
Library           ESP32GPIO.py

Suite Setup       Connect To Device    ${BAUDRATE}
Suite Teardown    Close Connection


*** Variables ***
${BAUDRATE}       115200


*** Test Cases ***
Device Responds To Ping
    ${response}=    Send Command    PING
    Should Be Equal    ${response}    PONG


Firmware Version Is Correct
    ${response}=    Send Command    GET_VERSION
    Should Be Equal    ${response}    VERSION:1.0.0


Device Reports Uptime
    ${response}=    Send Command    GET_UPTIME
    Should Match Regexp    ${response}    ^UPTIME:[0-9]+$


LED Blink Patterns
    # Approximately 4 cycles per second
    Repeat Keyword    20 times    Blink LED    0.125s

    # 2 cycles per second
    Repeat Keyword    10 times    Blink LED    0.25s

    # 1 cycle per second
    Repeat Keyword    5 times    Blink LED    0.5s

    ${state}=    Send Command    GET_LED
    Should Be Equal    ${state}    LED:OFF


LED Blinks Five Times Per Second
    ${count}=    Measure LED Pulses    1.0    5
    Should Be Equal As Integers    ${count}    5


LED Blinks Exactly Five Times
    ${count}=    Count LED Blinks    5
    Should Be Equal As Integers    ${count}    5


Unknown Command Is Rejected
    ${response}=    Send Command    INVALID_COMMAND
    Should Be Equal    ${response}    ERROR:UNKNOWN_COMMAND

GPIO Loopback Verification Passes
    ${response}=    Send Command    VERIFY_LOOPBACK
    Should Be Equal    ${response}    LOOPBACK:PASS

*** Keywords ***
Blink LED
    [Arguments]    ${half_period}
    Send Command    LED ON
    Sleep    ${half_period}
    Send Command    LED OFF
    Sleep    ${half_period}

