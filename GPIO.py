import RPi.GPIO as GPIO
import threading
import time
import queue

# GLOBALS
# I/Os
BUTTON1_PIN = 17 # e-stop
BUTTON2_PIN = 22 # calibrate
BUTTON3_PIN = 27 # takeoff

debounceDelay = 0.02

LEVER_PIN = 23

ARM_LED = 5
CONN_LED = 6

ARM_SWITCH = 23

# update on change
prev_button1_state = GPIO.HIGH
prev_button2_state = GPIO.HIGH
prev_button3_state = GPIO.HIGH
prev_lever_state = GPIO.HIGH

def setupGPIO():
	# Setup GPIO
	GPIO.setmode(GPIO.BCM)

	GPIO.setup(BUTTON1_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(BUTTON2_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(BUTTON3_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(LEVER_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	
	GPIO.setup(ARM_LED, GPIO.OUT, initial=GPIO.LOW)
	GPIO.setup(CONN_LED, GPIO.OUT, initial=GPIO.LOW)

	GPIO.setup(ARM_SWITCH, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def blink_leds():
	print("Starting LED blink loop")
	while True:
		GPIO.output(ARM_LED, GPIO.HIGH)
		GPIO.output(CONN_LED, GPIO.HIGH)
		time.sleep(1)

		GPIO.output(ARM_LED, GPIO.LOW)
		GPIO.output(CONN_LED, GPIO.LOW)
		time.sleep(1)

def read_inputs():
	
	global prev_button1_state, prev_button2_state, prev_button3_state, prev_lever_state, prevPotVal
	global armed_state

	prev_armed_state = GPIO.input(ARM_SWITCH)
	while True:
		
		button1_state = GPIO.input(BUTTON1_PIN)
		button2_state = GPIO.input(BUTTON2_PIN)
		button3_state = GPIO.input(BUTTON3_PIN)

		armed = GPIO.input(ARM_SWITCH)
		if armed != prev_armed_state:
			time.sleep(debounceDelay)
			prev_armed_state = armed
			
			if armed == GPIO.HIGH:
				print("ARMED")
			else:
				print("DISARMED")

		if button1_state != prev_button1_state:
			time.sleep(debounceDelay)
			prev_button1_state = button1_state
			if button1_state == GPIO.LOW:
				input_queue.put("E-STOP pressed")

		if button2_state != prev_button2_state:
			time.sleep(debounceDelay)
			prev_button2_state = button2_state
			if button2_state == GPIO.LOW:
				input_queue.put("CALIBRATE pressed")

		if button3_state != prev_button3_state:
			time.sleep(debounceDelay)
			prev_button3_state = button3_state
			if button3_state == GPIO.LOW:
				input_queue.put("TAKEOFF pressed")

		
def update_rgb_led():
		# todo
		pass

if __name__ == '__main__':

	input_queue = queue.Queue()
	setupGPIO()
	input_thread = threading.Thread(target=read_inputs)
	input_thread.start()

	led_thread = threading.Thread(target=blink_leds)
	led_thread.start()

	try:
		while True:
			try:
				input_response = input_queue.get(timeout=1)
				print("Received Input: ", input_response)

			except queue.Empty:
				pass

	except KeyboardInterrupt:
		GPIO.cleanup()


