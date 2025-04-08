package main

import (
	"bufio"
	"fmt"
	"log"
	"time"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
)

func findSerialPort() (string, error) {
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		return "", err
	}

	for _, port := range ports {
		if port.IsUSB {
			return port.Name, nil
		}
	}
	return "", fmt.Errorf("no suitable USB serial port found")
}

func main() {
	portName, err := findSerialPort()
	if err != nil {
		log.Fatal(err)
	}

	mode := &serial.Mode{
		BaudRate: 115200,
	}

	port, err := serial.Open(portName, mode)
	if err != nil {
		log.Fatal(err)
	}
	defer port.Close()

	fmt.Printf("Connected to serial port: %s\n", portName)

	writer := bufio.NewWriter(port)

	for {
		timeStr := fmt.Sprintf("UPTIME:%s\n", time.Now().Format("15:04:05"))
		writer.WriteString(timeStr)
		writer.Flush()

		ramStr := "RAM:76% (18.2GB/24GB)\n"
		writer.WriteString(ramStr)
		writer.Flush()

		netStr := "NET(U/D):102.5GB / 174.8GB\n"
		writer.WriteString(netStr)
		writer.Flush()

		diskStr := "DISK(R/W):3.2TB / 972.4GB\n"
		writer.WriteString(diskStr)
		writer.Flush()

		time.Sleep(5 * time.Second)
	}
}
