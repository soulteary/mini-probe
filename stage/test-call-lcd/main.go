package main

import (
	"bufio"
	"fmt"
	"log"
	"strings"
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
	reader := bufio.NewScanner(port)

	go func() {
		for reader.Scan() {
			line := reader.Text()
			fmt.Println("Received from device:", line)
			if strings.TrimSpace(line) == "SYNC_REQUEST" {
				response := fmt.Sprintf("NOW:%s\n", time.Now().Format("15:04:05"))
				writer.WriteString(response)
				writer.Flush()
				fmt.Println("Responded with time sync:", response)
			}
		}
	}()

	for {
		dataItems := []string{
			fmt.Sprintf("UPTIME:%s\n", "10days"),
			"RAM:76% (18.2GB/24GB)\n",
			"NET(U/D):102.5GB / 174.8GB\n",
			"DISK(R/W):3.2TB / 972.4GB\n",
			fmt.Sprintf("NOW:%s\n", time.Now().Format("15:04:05")),
		}

		for _, item := range dataItems {
			_, err := writer.WriteString(item)
			if err != nil {
				log.Printf("Failed to write to serial port: %v", err)
			}
			writer.Flush()
			time.Sleep(200 * time.Millisecond)
		}

		time.Sleep(1 * time.Second)
	}
}
