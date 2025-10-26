# FreeRTOS – Bài tập lập trình đa tác vụ (STM32F103)

## 🧩 Giới thiệu
Tài liệu này mô tả ba bài thực hành ứng dụng **FreeRTOS** trên **STM32F103**, tập trung vào việc **quản lý task, xử lý ngắt, và chia sẻ tài nguyên (UART, Mutex)**.  
Các bài học giúp sinh viên hiểu rõ cơ chế **task scheduling, synchronization và communication** trong hệ điều hành thời gian thực.

---

## 🧠 Bài 1 – Xử lý ngắt ngoài và Task Blink LED

### 🎯 Yêu cầu
- Viết chương trình có **2 task**:
  1. **Task Blink LED** – nhấp nháy LED định kỳ.  
  2. **Task xử lý ngắt (IRQ Handler Task)** – bật đèn hoặc còi cảnh báo khi có ngắt ngoài.  
- Khi **nhấn nút**, task xử lý được kích hoạt ngay sau khi hàm ngắt (ISR) thực thi.  
- Khi **chưa nhấn nút**, task xử lý ở trạng thái **Block** (treo chờ sự kiện).

### 🧩 Mục tiêu học tập
- Hiểu cơ chế **IRQ + Task Notification/Event** trong FreeRTOS.  
- Sử dụng **ngắt ngoài (EXTI)** để kích hoạt một task cụ thể.  
- Quản lý trạng thái Block/Ready của task hiệu quả.  

---

## 💬 Bài 2 – Gửi dữ liệu UART không dùng Mutex

### 🎯 Yêu cầu
- Viết chương trình có **2 task cùng gửi dữ liệu qua UART**.  
- **Không sử dụng Mutex** để bảo vệ tài nguyên UART.  
- Quan sát **sự xung đột dữ liệu** (data collision) trên terminal.


### 🧩 Mục tiêu học tập
- Hiểu vấn đề **race condition** và **data corruption** khi chia sẻ tài nguyên.  
- Thấy rõ tầm quan trọng của cơ chế **Mutex/Semaphore** trong FreeRTOS.

---

## 🔒 Bài 3 – Gửi dữ liệu UART có sử dụng Mutex

### 🎯 Yêu cầu
- Viết chương trình có **2 task cùng gửi dữ liệu qua UART**, **nhưng lần này sử dụng Mutex** để đảm bảo không xung đột.  
- Mutex giúp **chia sẻ tài nguyên UART an toàn** giữa các task.  

### ⚙️ Gợi ý thực hiện
- Tạo Mutex bằng `xSemaphoreCreateMutex()`.  
- Trước khi gửi dữ liệu:
  ```c
  xSemaphoreTake(uartMutex, portMAX_DELAY);
  UART_SendString("Hello from Task 1");
  xSemaphoreGive(uartMutex);
