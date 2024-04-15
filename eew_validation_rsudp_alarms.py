import socket
import json
import requests
import sys

# Define endpoint URLs for querying seismic data
url = "https://stationview.raspberryshake.org/query/objects.json?QC&GM"

# Extract primary and secondary station IDs from command-line arguments
if len(sys.argv) < 3:
    print("Usage: python program.py <primary_station_id> <secondary_station_id1> <secondary_station_id2> ...")
    sys.exit(1)

primary_station_id = sys.argv[1]
secondary_station_ids = sys.argv[2:]

# UDP server settings
UDP_IP = "127.0.0.1"
UDP_PORT = 12345
FORWARD_PORT = 54321  # Example forward port

# Create a UDP socket and bind the sedner and receiver socket to the port
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
forward_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print("UDP server started, waiting for ALARM messages...")

while True:
    # Receive UDP packet
    data, addr = sock.recvfrom(1024)
    message = data.decode().strip()

    # Check if the message starts with "ALARM"
    if message.startswith("ALARM"):
        print("Received ALARM message:", message)
        # Query seismic data for primary and secondary stations
        response = requests.get(url)
        seismic_data = response.json().get("request", {}).get("GM", {}).get("list", [])
        
        # Filter primary and secondary station data
        primary_data = [item for item in seismic_data if item["id"] == primary_station_id]
        secondary_data = [item for item in seismic_data if item["id"] in secondary_station_ids]
        
        # Extract seismic data from primary station
        primary_acc = primary_data[0].get("acc")
        primary_vel = primary_data[0].get("vel")
        primary_disp = primary_data[0].get("disp")

        # Extract seismic data from secondary stations and calculate averages
        avg_acc = sum(item.get("acc") for item in secondary_data) / len(secondary_data)
        avg_vel = sum(item.get("vel") for item in secondary_data) / len(secondary_data)
        avg_disp = sum(item.get("disp") for item in secondary_data) / len(secondary_data)

        # Compare each secondary station data with the primary station data
        if all([
            abs(avg_acc - primary_acc) / primary_acc <= 0.05,
            abs(avg_vel - primary_vel) / primary_vel <= 0.05,
            abs(avg_disp - primary_disp) / primary_disp <= 0.05
        ]):
            print("Data from secondary stations is within 0.05 of primary station data.")
            forward_sock.sendto(data, (UDP_IP, FORWARD_PORT))
            print("ALARM forwarded")
        else:
            print("Data from secondary stations is not within 0.05 of primary station data")
    else:
        print("Received unrecognized message:", message)
    print("Listening for incoming packet")