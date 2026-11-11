# BTL CO3009 – Hệ thống đèn giao thông 2 tuyến

**Môn:** CO3009 – Nhập môn Hệ thống nhúng  
**Nhóm:** <!-- điền số nhóm -->  
**Thành viên:**
| Tên | MSSV | Vai trò |
|-----|------|---------|
| <!-- TV1 --> | <!-- MSSV --> | Hạ tầng, Scheduler, fsm_manual |
| <!-- TV2 --> | <!-- MSSV --> | Phần cứng, LCD, DHT20 |
| <!-- TV3 --> | <!-- MSSV --> | Nút bấm, fsm_automatic, fsm_tuning |

---

## Mô tả dự án

Hệ thống điều khiển đèn giao thông 2 tuyến trên mạch **XNUCLEO-F103RB** (STM32F103RB), gồm 3 chế độ hoạt động:

- **Automatic**: Đèn tự động chuyển theo chu kỳ cài sẵn
- **Manual**: Người dùng điều khiển tuyến nào xanh bằng nút bấm
- **Tuning**: Chỉnh thời gian xanh/vàng/đỏ

Hiển thị chế độ, thời gian, nhiệt độ và độ ẩm lên màn hình LCD qua I2C. Cảm biến DHT20 đọc nhiệt độ và độ ẩm.

---

## Cấu trúc thư mục

```
BTL-CO3009-Den-Giao-Thong/
├── Core/
│   ├── Inc/
│   │   ├── global.h          # Biến dùng chung
│   │   ├── scheduler.h
│   │   ├── software_timer.h
│   │   ├── button.h
│   │   ├── traffic_light.h
│   │   ├── fsm_automatic.h
│   │   ├── fsm_manual.h
│   │   ├── fsm_tuning.h
│   │   ├── lcd.h
│   │   └── dht20.h
│   └── Src/
│       ├── main.c
│       ├── global.c
│       ├── scheduler.c
│       ├── software_timer.c
│       ├── button.c
│       ├── traffic_light.c
│       ├── fsm_automatic.c
│       ├── fsm_manual.c
│       ├── fsm_tuning.c
│       ├── lcd.c
│       └── dht20.c
└── README.md
```

---

## Hướng dẫn build và nạp

### Yêu cầu
- **STM32CubeIDE** (phiên bản 1.13 trở lên)
- Mạch **XNUCLEO-F103RB** kết nối qua cáp USB
- Module đèn giao thông ×2, module nút bấm ×2, LCD I2C, cảm biến DHT20

### Build
1. Mở STM32CubeIDE
2. **File → Import → Existing Projects into Workspace**
3. Chọn thư mục repo này
4. Click **Build** (Ctrl+B)

### Nạp vào mạch
> ⚠️ Đảm bảo đã cấu hình **SYS → Debug = Serial Wire** trong file `.ioc` trước khi nạp lần đầu!

1. Kết nối mạch XNUCLEO-F103RB qua USB
2. Click **Run** (F11) hoặc chọn **Run → Run As → STM32 C/C++ Application**

---

## Phân công và mốc thời gian

| Deadline | Việc | Người |
|----------|------|-------|
| 12/11 | Repo GitHub + dự án CubeIDE + `.ioc` | TV1 |
| 16/11 | Scheduler O(1) + Software Timer | TV1 |
| 18/11 | Driver LCD | TV2 |
| 18/11 | Module button + traffic_light | TV3 |
| 20/11 | Driver DHT20 | TV2 |
| 20/11 | fsm_automatic | TV3 |
| 21/11 | fsm_manual + chuyển chế độ | TV1 |
| 22/11 | Màn hình LCD cho từng chế độ | TV2 |
| 22/11 | fsm_tuning | TV3 |
| **22/11** | **Nạp lần đầu vào mạch (hạn chung)** | **Cả nhóm** |
| 3/12 | Báo cáo + README + tag v1.0-final | Cả nhóm |
| **8/12** | **Nộp chính thức** | — |

---

## Quy ước code

- **Tên file:** chữ thường, gạch dưới → `fsm_automatic.c`
- **Tên hàm:** `<module>_<hành_động>` → `button_read()`, `scheduler_init()`
- **HAL** chỉ nằm trong module phần cứng và `main.c`; các file `fsm_*.c` **không gọi HAL**
- **Không dùng** `HAL_Delay` trong chương trình chính
- Mỗi người làm nhánh `feature/tên-mình`, gộp vào `dev`, cuối cùng vào `main`
- Thông điệp commit rõ ràng: `scheduler: add one-shot task support`

---

## Commit hash nộp bài

Tag: `v1.0-final`  
Commit hash: <!-- điền sau khi tag -->

---

## Link LMS

<!-- điền link đăng ký nhóm trên LMS -->
