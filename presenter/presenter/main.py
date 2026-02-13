import json
import sys
import argparse
import logging
import paho.mqtt.client as mqtt

# Configure logger
logger = logging.getLogger(__name__)
log_format = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

def configure_logging(log_file=None):
    """Configure logger with console and optional file handlers"""
    logger.setLevel(logging.INFO)
    
    # Console handler
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_handler.setFormatter(log_format)
    logger.addHandler(console_handler)
    
    # File handler (if log_file specified)
    if log_file:
        try:
            file_handler = logging.FileHandler(log_file)
            file_handler.setLevel(logging.INFO)
            file_handler.setFormatter(log_format)
            logger.addHandler(file_handler)
            logger.info(f"Logging to file: {log_file}")
        except Exception as e:
            logger.error(f"Failed to open log file {log_file}: {e}")

def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        logger.info("Connected successfully. Subscribing to topics...")
        for topic in userdata['topics']:
            client.subscribe(topic)
            logger.info(f"Subscribed to: {topic}")
    else:
        logger.error(f"Connection failed with code {rc}")

def on_message(client, userdata, msg):
    # This logs to both console and file
    logger.info(f"[{msg.topic}] {msg.payload.decode('utf-8', 'ignore')}")

def main():
    parser = argparse.ArgumentParser(description="MQTT Presenter")
    parser.add_argument("--config", "-c", required=True, help="Path to config file")
    args = parser.parse_args()
    
    try:
        with open(args.config, 'r') as f:
            config = json.load(f)
    except Exception as e:
        logger.error(f"Error loading config: {e}")
        sys.exit(1)

    # Configure logging with optional log file from config
    log_file = config['presenter'].get('log_file', None)
    configure_logging(log_file)

    # Use CallbackAPIVersion.VERSION2 for Python 3.12 compatibility
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    
    # Pass topics to callbacks via userdata
    client.user_data_set(config['presenter'])
    
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(config['presenter']['broker'], config['presenter'].get('port', 1883), 60)
    
    logger.info("Starting MQTT Listener...")
    client.loop_forever()

if __name__ == "__main__":
    main()