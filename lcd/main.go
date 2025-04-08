package main

import (
	"bufio"
	"fmt"
	"log"
	"strings"
	"time"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/host"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/net"
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

func formatBytes(b uint64) string {
	const unit = 1024
	if b < unit {
		return fmt.Sprintf("%d B", b)
	}
	div, exp := unit, 0
	for n := b / unit; n >= unit; n /= unit {
		div *= unit
		exp++
	}
	return fmt.Sprintf("%.2f %cB", float64(b)/float64(div), "KMGTPE"[exp])
}

func collectSystemInfo() ([]string, error) {
	bootTime, err := host.BootTime()
	if err != nil {
		return nil, err
	}
	uptime := time.Since(time.Unix(int64(bootTime), 0)).Round(time.Second)

	vmStat, err := mem.VirtualMemory()
	if err != nil {
		return nil, err
	}

	netIOCounters, err := net.IOCounters(false)
	if err != nil || len(netIOCounters) == 0 {
		return nil, fmt.Errorf("failed to get network data")
	}
	netIO := netIOCounters[0]

	diskIOCounters, err := disk.IOCounters()
	if err != nil {
		return nil, err
	}
	var readBytes, writeBytes uint64
	for _, io := range diskIOCounters {
		readBytes += io.ReadBytes
		writeBytes += io.WriteBytes
	}

	dataItems := []string{
		fmt.Sprintf("UPTIME:%v\n", uptime),
		fmt.Sprintf("RAM:%.2f%% (%s/%s)\n", vmStat.UsedPercent, formatBytes(vmStat.Used), formatBytes(vmStat.Total)),
		fmt.Sprintf("NET(U/D):%s / %s\n", formatBytes(netIO.BytesSent), formatBytes(netIO.BytesRecv)),
		fmt.Sprintf("DISK(R/W):%s / %s\n", formatBytes(readBytes), formatBytes(writeBytes)),
		fmt.Sprintf("NOW:%s\n", time.Now().Format("15:04:05")),
	}

	return dataItems, nil
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
		dataItems, err := collectSystemInfo()
		if err != nil {
			log.Printf("Error collecting system info: %v", err)
			continue
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