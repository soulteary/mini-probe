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
	// 获取系统运行时间
	bootTime, err := host.BootTime()
	if err != nil {
		http.Error(w, "无法获取系统启动时间", http.StatusInternalServerError)
		return
	}
	uptime := time.Since(time.Unix(int64(bootTime), 0)).Round(time.Second)

	// 获取内存使用情况
	vmStat, err := mem.VirtualMemory()
	if err != nil {
		http.Error(w, "无法获取内存信息", http.StatusInternalServerError)
		return
	}

	// 获取网络上传下载数据
	netIOCounters, err := net.IOCounters(false)
	if err != nil || len(netIOCounters) == 0 {
		http.Error(w, "无法获取网络数据", http.StatusInternalServerError)
		return
	}
	netIO := netIOCounters[0]

	// 获取磁盘读写数据
	diskIOCounters, err := disk.IOCounters()
	if err != nil {
		http.Error(w, "无法获取磁盘数据", http.StatusInternalServerError)
		return
	}
	var readBytes, writeBytes uint64
	for _, io := range diskIOCounters {
		readBytes += io.ReadBytes
		writeBytes += io.WriteBytes
	}

	fmt.Fprintf(w, "服务器运行时间: %v\n", uptime)
	fmt.Fprintf(w, "内存使用: 已使用 %.2f%% (%v/%v)\n", vmStat.UsedPercent, formatBytes(vmStat.Used), formatBytes(vmStat.Total))
	fmt.Fprintf(w, "网络上传数据: %v, 下载数据: %v\n", formatBytes(netIO.BytesSent), formatBytes(netIO.BytesRecv))
	fmt.Fprintf(w, "磁盘读取数据: %v, 写入数据: %v\n", formatBytes(readBytes), formatBytes(writeBytes))
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

func main() {
	http.HandleFunc("/probe", probeHandler)
	fmt.Println("服务器探针运行在 :8080/probe")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
