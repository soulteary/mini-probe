FROM golang:1.24-alpine AS builder
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
RUN echo '' > /etc/apk/repositories && \
    echo "https://mirror.tuna.tsinghua.edu.cn/alpine/v3.10/main"         >> /etc/apk/repositories && \
    echo "https://mirror.tuna.tsinghua.edu.cn/alpine/v3.10/community"    >> /etc/apk/repositories && \
    echo "Asia/Shanghai" > /etc/timezone
RUN apk add --no-cache gcc musl-dev linux-headers
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod tidy
COPY . .
RUN go build -o serial_monitor .


FROM alpine:latest
RUN apk add --no-cache libusb-dev
WORKDIR /app
COPY --from=builder /app/serial_monitor .
CMD ["./serial_monitor"]