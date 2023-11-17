import RPi.GPIO as GPIO
import threading
import time
import queue

# GLOBALS
# I/Os
BUTTON1_PIN = 11 # e-stop
BUTTON2_PIN = 15 # calibrate
BUTTON3_PIN = 13 # takeoff

debounceDelay = 0.02

LEVER_PIN = 23
POTENTIOMETER_PIN = 24

ARM_LED = 29
CONN_LED = 31
#RED_LED_PIN = 24
#GREEN_LED_PIN = 25
#BLUE_LED_PIN = 4

# update on change
prev_button1_state = GPIO.HIGH
prev_button2_state = GPIO.HIGH
prev_button3_state = GPIO.HIGH
prev_lever_state = GPIO.HIGH
prevPotVal = -1

potVal = 0.0

def setupGPIO():
	# Setup GPIO
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(BUTTON1_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(BUTTON2_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(BUTTON3_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(LEVER_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	
	GPIO.setup(ARM_LED, GPIO.OUT, initial=GPIO.LOW)
	GPIO.setup(CONN_LED, GPIO.OUT, initial=GPIO.LOW)

def blink_leds():
	print("Starting LED blink loop")
	while True:
		GPIO.output(ARM_LED, GPIO.HIGH)
		GPIO.output(CONN_LED, GPIO.HIGH)

		time.sleep(1)

		GPIO.output(ARM_LED, GPIO.HIGH)
		GPIO.output(CONN_LED, GPIO.HIGH)

		time.sleep(1)

def read_inputs():
	
	global prev_button1_state, prev_button2_state, prev_button3_state, prev_lever_state, prevPotVal
	
	while True:
		
		button1_state = GPIO.input(BUTTON1_PIN)
		button2_state = GPIO.input(BUTTON2_PIN)
		button3_state = GPIO.input(BUTTON3_PIN)
		lever_state = GPIO.input(LEVER_PIN)
		potentiometer_value = GPIO.input(POTENTIOMETER_PIN)
		

		if button1_state != prev_button1_state:
			time.sleep(debounceDelay)
			prev_button1_state = button1_state
			if button1_state == GPIO.LOW:
				input_queue.put("Button 1 is pressed")

		if button2_state != prev_button2_state:
			time.sleep(debounceDelay)
			prev_button2_state = button2_state
			if button2_state == GPIO.LOW:
				input_queue.put("Button 2 is pressed")

		if button3_state != prev_button3_state:
			time.sleep(debounceDelay)
			prev_button3_state = button3_state
			if button3_state == GPIO.LOW:
				input_queue.put("Button 3 is pressed")

		if lever_state != prev_lever_state:
			prev_lever_state = lever_state
			if lever_state == GPIO.LOW:
				input_queue.put("Lever is ON")
			if lever_state == GPIO.HIGH:
				input_queue.put("Lever is OFF")


		if potVal != prevPotVal:
			prevPotVal = potVal
			input_queue.put(f"Pot Value: {potVal}") 
	
	
		
		
def update_rgb_led():
		# todo
		pass

if __name__ == '__main__':

	input_queue = queue.Queue()
	setupGPIO()
	input_thread = threading.Thread(target=read_inputs)
	input_thread.start()

	led_thread = threading.Thread(target=blink_leds)

	try:
		while True:
			try:
				input_response = input_queue.get(timeout=1)
				print("Received Input: ", input_response)

			except queue.Empty:
				pass

	except KeyboardInterrupt:
		GPIO.cleanup()


