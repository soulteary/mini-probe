package main

import (
	"fmt"
	"log"
	"net/http"
	"time"

	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/host"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/net"
)

func probeHandler(w http.ResponseWriter, r *http.Request) {
	bootTime, err := host.BootTime()
	if err != nil {
		http.Error(w, "Error", http.StatusInternalServerError)
		return
	}
	uptime := time.Since(time.Unix(int64(bootTime), 0)).Round(time.Minute)

	vmStat, err := mem.VirtualMemory()
	if err != nil {
		http.Error(w, "Error", http.StatusInternalServerError)
		return
	}

	netIOCounters, err := net.IOCounters(false)
	if err != nil || len(netIOCounters) == 0 {
		http.Error(w, "Error", http.StatusInternalServerError)
		return
	}
	netIO := netIOCounters[0]

	diskIOCounters, err := disk.IOCounters()
	if err != nil {
		http.Error(w, "Error", http.StatusInternalServerError)
		return
	}
	var readBytes, writeBytes uint64
	for _, io := range diskIOCounters {
		readBytes += io.ReadBytes
		writeBytes += io.WriteBytes
	}

	fmt.Fprintf(w, "Server Uptime: %v\n", uptime)
	fmt.Fprintf(w, "Memory Usage: %.2f%% (%v/%v)\n", vmStat.UsedPercent, formatBytes(vmStat.Used), formatBytes(vmStat.Total))
	fmt.Fprintf(w, "Net Uploaded: %v, Downloaded: %v\n", formatBytes(netIO.BytesSent), formatBytes(netIO.BytesRecv))
	fmt.Fprintf(w, "Disk Read: %v, Written: %v\n", formatBytes(readBytes), formatBytes(writeBytes))
}

func formatBytes(b uint64) string {
	const unit = 1024
	if b < unit {
		return fmt.Sprintf("%dB", b)
	}
	div, exp := unit, 0
	for n := b / unit; n >= unit; n /= unit {
		div *= unit
		exp++
	}
	return fmt.Sprintf("%.1f%cB", float64(b)/float64(div), "KMGTPE"[exp])
}

func main() {
	http.HandleFunc("/probe", probeHandler)
	fmt.Println("Server Probe at :8080/probe")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
