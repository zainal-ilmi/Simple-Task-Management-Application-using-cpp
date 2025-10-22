#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <ctime>
#include <iomanip>
#include <algorithm>

#ifdef _WIN32
    #include <windows.h> 
#else
    #include <unistd.h>
#endif

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Menampilkan header 
void displayHeader() {
    time_t now = time(0);
    struct tm* timeInfo = localtime(&now);
    const char* days[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
    const char* months[] = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
    char timeStr[50];
    strftime(timeStr, sizeof(timeStr), "%H:%M", timeInfo);

    std::cout << CYAN << "=====================================\n";
    std::cout << "|       Aplikasi Manajemen Tugas    |\n";
    std::cout << "=====================================\n";
    std::cout << BLUE << "Tanggal: " << days[timeInfo->tm_wday] << ", " << timeInfo->tm_mday << " " 
              << months[timeInfo->tm_mon] << " " << (timeInfo->tm_year + 1900) << ", " 
              << timeStr << " WIB\n" << RESET;
    std::cout << "-------------------------------------\n";
}

void waitForEnter() {
    std::cout << YELLOW << "Tekan Enter untuk kembali ke menu..." << RESET << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Menampilkan animasi loading 
void showLoading() {
    std::cout << CYAN << "Loading [";
    for (int i = 0; i < 5; i++) {
        std::cout << "█" << std::flush;
        #ifdef _WIN32
            Sleep(300);
        #else
            usleep(300000);
        #endif
    }
    std::cout << "]" << RESET << "\n";
}

// Fungsi untuk mencatat log aktivitas
void logActivity(const std::string& message) {
    std::ofstream logFile("data/activity.log", std::ios::app);
    if (logFile.is_open()) {
        time_t now = time(0);
        std::string timestamp = ctime(&now);
        timestamp.pop_back();
        logFile << "[" << timestamp << "] " << message << std::endl;
        logFile.close();
    }
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Fungsi untuk validasi format tanggal (DD-MM-YYYY)
bool isValidDate(const std::string& date) {
    if (date.length() != 10 || date[2] != '-' || date[5] != '-') return false;

    std::stringstream ss(date);
    std::string day, month, year;
    std::getline(ss, day, '-');
    std::getline(ss, month, '-');
    std::getline(ss, year, '-');

    try {
        int d = std::stoi(day);
        int m = std::stoi(month);
        int y = std::stoi(year);

        if (y < 2025) return false;
        if (m < 1 || m > 12) return false;

        int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (m == 2 && isLeapYear(y)) daysInMonth[1] = 29;

        if (d < 1 || d > daysInMonth[m - 1]) return false;
        return true;
    } catch (...) {
        return false;
    }
}

// Fungsi untuk menghitung selisih hari
int calculateDaysDifference(const std::string& deadline) {
    time_t now = time(0);
    struct tm* currentDate = localtime(&now);

    struct tm deadlineDate = {};
    std::stringstream ss(deadline);
    std::string day, month, year;
    std::getline(ss, day, '-');
    std::getline(ss, month, '-');
    std::getline(ss, year, '-');
    deadlineDate.tm_mday = std::stoi(day);
    deadlineDate.tm_mon = std::stoi(month) - 1;
    deadlineDate.tm_year = std::stoi(year) - 1900;

    time_t currentTime = mktime(currentDate);
    time_t deadlineTime = mktime(&deadlineDate);

    double diffSeconds = difftime(deadlineTime, currentTime);
    int diffDays = static_cast<int>(diffSeconds / (60 * 60 * 24));
    return diffDays;
}

class User {
protected:
    std::string username;
    std::string password;

public:
    User(const std::string& uname = "", const std::string& pwd = "") 
        : username(uname), password(pwd) {}

    std::string getUsername() const { return username; }

    void saveToFile() const {
        std::ofstream file("data/users.csv", std::ios::app);
        if (file.is_open()) {
            file << username << "," << password << std::endl;
            file.close();
        } else {
            std::cout << RED << "[!] Gagal menyimpan data pengguna!\n" << RESET;
        }
    }

    // Memverifikasi autentikasi pengguna
    bool authenticate(const std::string& uname, const std::string& pwd) const {
        std::string cleanUname = uname, cleanPwd = pwd;
        while (cleanUname.front() == ' ' || cleanUname.front() == '\n' || cleanUname.front() == '\r') cleanUname.erase(0, 1);
        while (cleanUname.back() == ' ' || cleanUname.back() == '\n' || cleanUname.back() == '\r') cleanUname.pop_back();
        while (cleanPwd.front() == ' ' || cleanPwd.front() == '\n' || cleanPwd.front() == '\r') cleanPwd.erase(0, 1);
        while (cleanPwd.back() == ' ' || cleanPwd.back() == '\n' || cleanPwd.back() == '\r') cleanPwd.pop_back();

        return (cleanUname == username && cleanPwd == password);
    }
};

struct Task {
    std::string name;
    std::string course;
    std::string deadline;
    std::string status;

    Task(const std::string& n, const std::string& c, const std::string& d, const std::string& s = "Belum")
        : name(n), course(c), deadline(d), status(s) {}
};

class TaskManager : public User {
private:
    std::vector<Task> tasks;
    int notificationDays = 3; 

    // Memuat tugas dari file berdasarkan username
    void loadTasks() {
        tasks.clear();
        std::string filename = "data/tasks_" + username + ".csv";
        std::ifstream file(filename);
        std::string line;
        if (file.is_open()) {
            while (std::getline(file, line)) {
                if (!line.empty()) {
                    std::stringstream ss(line);
                    std::string name, course, deadline, status;
                    std::getline(ss, name, ',');
                    std::getline(ss, course, ',');
                    std::getline(ss, deadline, ',');
                    std::getline(ss, status, ',');
                    tasks.emplace_back(name, course, deadline, status);
                }
            }
            file.close();
        }

        // Fungsi mengatur notifikasi user
        std::string configFile = "data/config_" + username + ".txt";
        std::ifstream config(configFile);
        if (config.is_open() && std::getline(config, line)) {
            try {
                notificationDays = std::stoi(line);
                if (notificationDays < 1) notificationDays = 3; 
            } catch (...) {
                notificationDays = 3; 
            }
            config.close();
        }
    }

    // Fungsi Menyimpan tugas 
    void saveTasks() const {
        std::string filename = "data/tasks_" + username + ".csv";
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const Task& task : tasks) {
                file << task.name << "," << task.course << "," << task.deadline << "," << task.status << std::endl;
            }
            file.close();
        } else {
            std::cout << RED << "[!] Gagal menyimpan data tugas!\n" << RESET;
        }
    }

    // Fungsi menyimpan batas notifikasi 
    void saveNotificationDays() const {
        std::string configFile = "data/config_" + username + ".txt";
        std::ofstream config(configFile);
        if (config.is_open()) {
            config << notificationDays << std::endl;
            config.close();
        }
    }

    // Menampilkan tugas dalam tabel
    void displayTasks(const std::vector<Task>& tasksToDisplay) const {
        if (tasksToDisplay.empty()) {
            std::cout << RED << "[!] Tidak ada tugas yang cocok.\n" << RESET;
            waitForEnter();
            return;
        }

        const int noWidth = 5;
        const int nameWidth = 20;
        const int courseWidth = 25;
        const int deadlineWidth = 15;
        const int statusWidth = 12;

        std::string separator = "+" + std::string(noWidth, '-') + "+" + std::string(nameWidth, '-') + "+" + 
                                std::string(courseWidth, '-') + "+" + std::string(deadlineWidth, '-') + "+" + 
                                std::string(statusWidth, '-') + "+";

        std::cout << separator << "\n";
        std::cout << "|" << std::setw(noWidth) << std::left << " No " << "|" 
                  << std::setw(nameWidth) << " Nama Tugas " << "|" 
                  << std::setw(courseWidth) << " Nama Mata Kuliah " << "|" 
                  << std::setw(deadlineWidth) << " Tenggat " << "|" 
                  << std::setw(statusWidth) << " Status " << "|\n";
        std::cout << separator << "\n";

        for (size_t i = 0; i < tasksToDisplay.size(); ++i) {
            std::string statusDisplay = tasksToDisplay[i].status == "Belum" ? "❌ Belum" : "✅ Selesai";
            std::cout << "|" << std::setw(noWidth - 1) << std::left << (i + 1) << " |" 
                      << std::setw(nameWidth - 1) << (tasksToDisplay[i].name.length() > nameWidth - 2 ? tasksToDisplay[i].name.substr(0, nameWidth - 5) + "..." : tasksToDisplay[i].name) << " |" 
                      << std::setw(courseWidth - 1) << (tasksToDisplay[i].course.length() > courseWidth - 2 ? tasksToDisplay[i].course.substr(0, courseWidth - 5) + "..." : tasksToDisplay[i].course) << " |" 
                      << std::setw(deadlineWidth - 1) << tasksToDisplay[i].deadline << " |";
            if (tasksToDisplay[i].status == "Belum") {
                std::cout << RED << std::setw(statusWidth - 1) << statusDisplay << RESET << " |";
            } else {
                std::cout << GREEN << std::setw(statusWidth - 1) << statusDisplay << RESET << " |";
            }
            std::cout << "\n";
        }

        std::cout << separator << "\n";
        waitForEnter();
    }

public:
    TaskManager(const std::string& uname, const std::string& pwd) 
        : User(uname, pwd) {
        loadTasks();
    }
    

    ~TaskManager() {
        saveTasks();
        saveNotificationDays();
        std::cout << GREEN << "[*] Memori untuk tugas pengguna " << username << " dibebaskan.\n" << RESET;
    }

    void displayMenu() const {
        clearScreen();
        displayHeader();
        std::cout << BLUE << "Selamat datang, " << username << "!\n\n" << RESET;
        std::cout << CYAN << "===== Menu Pengguna =====" << RESET << "\n";
        std::cout << "| " << MAGENTA << BOLD << "1. Tambah Tugas     📝" << RESET << "  |\n";
        std::cout << "| " << MAGENTA << BOLD << "2. Lihat Tugas      📋" << RESET << "  |\n";
        std::cout << "| " << MAGENTA << BOLD << "3. Edit Tugas       ✏️" << RESET << "  |\n";
        std::cout << "| " << MAGENTA << BOLD << "4. Hapus Tugas      🗑️" << RESET << "  |\n";
        std::cout << "| " << MAGENTA << BOLD << "5. Cari/Filter Tugas 🔍" << RESET << " |\n";
        std::cout << "| " << MAGENTA << BOLD << "6. Atur Notifikasi  ⏰" << RESET << "  |\n";
        std::cout << "| " << MAGENTA << BOLD << "7. Keluar           🚪" << RESET << "  |\n";
        std::cout << CYAN << "========================" << RESET << "\n";
        std::cout << "Pilih opsi [1-7]: ";
    }

    // Fungsi menambahkan tugas baru
    void addTask() {
        clearScreen();
        displayHeader();
        std::cout << BLUE << "=== Tambah Tugas ===\n\n" << RESET;

        std::string name, course, deadline;
        std::cout << "Masukkan Nama Tugas: ";
        std::cin.sync();
        std::getline(std::cin, name);
        if (name.empty()) {
            std::cout << RED << "[!] Nama Tugas tidak boleh kosong!\n" << RESET;
            waitForEnter();
            return;
        }

        std::cout << "Masukkan Nama Mata Kuliah: ";
        std::cin.sync();
        std::getline(std::cin, course);
        if (course.empty()) {
            std::cout << RED << "[!] Nama Mata Kuliah tidak boleh kosong!\n" << RESET;
            waitForEnter();
            return;
        }

        std::cout << "Masukkan Tenggat (format: DD-MM-YYYY): ";
        std::cin.sync();
        std::getline(std::cin, deadline);
        if (deadline.empty()) {
            std::cout << RED << "[!] Tenggat tidak boleh kosong!\n" << RESET;
            waitForEnter();
            return;
        }

        if (!isValidDate(deadline)) {
            std::cout << RED << "[!] Format tanggal tidak valid. Gunakan DD-MM-YYYY (contoh: 04-06-2025).\n" << RESET;
            waitForEnter();
            return;
        }

        tasks.emplace_back(name, course, deadline, "Belum");
        saveTasks();
        std::cout << GREEN << "\n[*] Tugas berhasil ditambahkan!\n" << RESET;
        waitForEnter();
    }

    void viewTasks() const {
        clearScreen();
        displayHeader();
        std::cout << BLUE << "=== Daftar Tugas ===\n\n" << RESET;
        displayTasks(tasks);
    }

    void editTask() {
        clearScreen();
        displayHeader();
        std::cout << BLUE << "=== Edit Tugas ===\n\n" << RESET;

        if (tasks.empty()) {
            std::cout << RED << "[!] Tidak ada tugas yang tersedia.\n" << RESET;
            waitForEnter();
            return;
        }
// display task function

        const int noWidth = 5;
        const int nameWidth = 20;
        const int courseWidth = 25;
        const int deadlineWidth = 15;
        const int statusWidth = 12; 
        std::string separator = "+" + std::string(noWidth, '-') + "+" + std::string(nameWidth, '-') + "+" + 
                                std::string(courseWidth, '-') + "+" + std::string(deadlineWidth, '-') + "+" + 
                                std::string(statusWidth, '-') + "+";
        std::cout << separator << "\n";
        std::cout << "|" << std::setw(noWidth) << std::left << " No " << "|" 
                  << std::setw(nameWidth) << " Nama Tugas " << "|" 
                  << std::setw(courseWidth) << " Nama Mata Kuliah " << "|" 
                  << std::setw(deadlineWidth) << " Tenggat " << "|" 
                  << std::setw(statusWidth) << " Status " << "|\n";
        std::cout << separator << "\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            std::string statusDisplay = tasks[i].status == "Belum" ? "❌ Belum" : "✅ Selesai";
            std::cout << "|" << std::setw(noWidth - 1) << std::left << (i + 1) << " |" 
                      << std::setw(nameWidth - 1) << (tasks[i].name.length() > nameWidth - 2 ? tasks[i].name.substr(0, nameWidth - 5) + "..." : tasks[i].name) << " |" 
                      << std::setw(courseWidth - 1) << (tasks[i].course.length() > courseWidth - 2 ? tasks[i].course.substr(0, courseWidth - 5) + "..." : tasks[i].course) << " |" 
                      << std::setw(deadlineWidth - 1) << tasks[i].deadline << " |";
            if (tasks[i].status == "Belum") {
                std::cout << RED << std::setw(statusWidth - 1) << statusDisplay << RESET << " |";
            } else {
                std::cout << GREEN << std::setw(statusWidth - 1) << statusDisplay << RESET << " |";
            }
            std::cout << "\n";
        }
        std::cout << separator << "\n";

        //edit task
        
        int index;
        std::cout << "\nMasukkan nomor tugas yang ingin diedit (1-" << tasks.size() << "): ";
        if (!(std::cin >> index)) {
            std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            waitForEnter();
            return;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (index < 1 || static_cast<size_t>(index) > tasks.size()) {
            std::cout << RED << "\n[!] Nomor tugas tidak valid!\n" << RESET;
            waitForEnter();
            return;
        }

        int choice;
        do {
            clearScreen();
            displayHeader();
            std::cout << BLUE << "=== Submenu Edit Tugas ===\n\n" << RESET;
            std::cout << "Tugas yang dipilih: " << tasks[index - 1].name << "\n\n";
            std::cout << CYAN << "===== Opsi Edit =====" << RESET << "\n";
            std::cout << "| " << MAGENTA << BOLD << "1. Edit Tugas     ✏️" << RESET << " |\n";
            std::cout << "| " << MAGENTA << BOLD << "2. Edit Status    ✅" << RESET << " |\n";
            std::cout << "| " << MAGENTA << BOLD << "3. Kembali        ⬅️" << RESET << " |\n";
            std::cout << CYAN << "=====================" << RESET << "\n";
            std::cout << "Pilih opsi [1-3]: ";
            if (!(std::cin >> choice)) {
                std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                waitForEnter();
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (choice == 1) {
                std::string name, course, deadline;
                std::cout << "\nMasukkan Nama Tugas baru (kosongkan untuk tidak mengubah): ";
                std::cin.sync();
                std::getline(std::cin, name);
                std::cout << "Masukkan Nama Mata Kuliah baru (kosongkan untuk tidak mengubah): ";
                std::cin.sync();
                std::getline(std::cin, course);
                std::cout << "Masukkan Tenggat baru (format: DD-MM-YYYY, kosongkan untuk tidak mengubah): ";
                std::cin.sync();
                std::getline(std::cin, deadline);

                if (!deadline.empty() && !isValidDate(deadline)) {
                    std::cout << RED << "[!] Format tanggal tidak valid. Gunakan DD-MM-YYYY (contoh: 04-06-2025).\n" << RESET;
                    waitForEnter();
                    continue;
                }

                tasks[index - 1] = Task(name.empty() ? tasks[index - 1].name : name,
                                        course.empty() ? tasks[index - 1].course : course,
                                        deadline.empty() ? tasks[index - 1].deadline : deadline,
                                        tasks[index - 1].status);
                saveTasks();
                std::cout << GREEN << "\n[*] Tugas berhasil diedit!\n" << RESET;
                waitForEnter();
            } else if (choice == 2) {
                std::string status;
                std::cout << "\nMasukkan Status baru (Belum/Selesai): ";
                std::cin.sync();
                std::getline(std::cin, status);
                if (status != "Belum" && status != "Selesai") {
                    std::cout << RED << "\n[!] Status hanya boleh 'Belum' atau 'Selesai'!\n" << RESET;
                } else {
                    tasks[index - 1].status = status;
                    saveTasks();
                    std::cout << GREEN << "\n[*] Status berhasil diedit!\n" << RESET;
                    // Perbarui notifikasi setelah status diubah
                    showDeadlineNotifications();
                }
                waitForEnter();
            } else if (choice == 3) {
                std::cout << GREEN << "\n[*] Kembali ke menu tugas.\n" << RESET;
                waitForEnter();
                break;
            } else {
                std::cout << RED << "\n[!] Pilihan tidak valid!\n" << RESET;
                waitForEnter();
            }
        } while (true);
    }

    // Fungsi menghapus tugas tertentu
    void deleteTask() {
        clearScreen();
        displayHeader();
        std::cout << BLUE << "=== Hapus Tugas ===\n\n" << RESET;
        viewTasks();
        if (!tasks.empty()) {
            int index;
            std::cout << "Masukkan nomor tugas yang ingin dihapus (1-" << tasks.size() << "): ";
            if (!(std::cin >> index)) {
                std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                waitForEnter();
                return;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (index > 0 && static_cast<size_t>(index) <= tasks.size()) {
                std::cout << "Apakah Anda yakin ingin menghapus tugas '" << tasks[index - 1].name << "'? (y/n): ";
                char confirm;
                std::cin >> confirm;
                if (confirm == 'y' || confirm == 'Y') {
                    tasks.erase(tasks.begin() + (index - 1));
                    saveTasks();
                    std::cout << GREEN << "\n[*] Tugas berhasil dihapus!\n" << RESET;
                } else {
                    std::cout << GREEN << "\n[*] Penghapusan dibatalkan.\n" << RESET;
                }
            } else {
                std::cout << RED << "\n[!] Nomor tugas tidak valid!\n" << RESET;
            }
        }
        waitForEnter();
    }

    // Fungsi mencari dan memfilter tugas
    void searchFilterTasks() const {
        int choice;
        do {
            clearScreen();
            displayHeader();
            std::cout << BLUE << "=== Cari/Filter Tugas ===\n\n" << RESET;
            std::cout << CYAN << "===== Opsi Filter =====" << RESET << "\n";
            std::cout << "| " << MAGENTA << BOLD << "1. Berdasarkan Nama Tugas  🔍" << RESET << " |\n";
            std::cout << "| " << MAGENTA << BOLD << "2. Berdasarkan Status      ✅" << RESET << " |\n";
            std::cout << "| " << MAGENTA << BOLD << "3. Berdasarkan Mata Kuliah 📚" << RESET << " |\n";
            std::cout << "| " << MAGENTA << BOLD << "4. Kembali                 ⬅️" << RESET << " |\n";
            std::cout << CYAN << "=======================" << RESET << "\n";
            std::cout << "Pilih opsi [1-4]: ";
            if (!(std::cin >> choice)) {
                std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                waitForEnter();
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::vector<Task> filteredTasks;
            if (choice == 1) {
                std::string keyword;
                std::cout << "\nMasukkan kata kunci Nama Tugas: ";
                std::cin.sync();
                std::getline(std::cin, keyword);
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
                for (const Task& task : tasks) {
                    std::string taskName = task.name;
                    std::transform(taskName.begin(), taskName.end(), taskName.begin(), ::tolower);
                    if (taskName.find(keyword) != std::string::npos) {
                        filteredTasks.push_back(task);
                    }
                }
                clearScreen();
                displayHeader();
                std::cout << BLUE << "=== Hasil Pencarian ===\n\n" << RESET;
                displayTasks(filteredTasks);
            } else if (choice == 2) {
                std::string status;
                std::cout << "\nMasukkan Status (Belum/Selesai): ";
                std::cin.sync();
                std::getline(std::cin, status);
                if (status != "Belum" && status != "Selesai") {
                    std::cout << RED << "\n[!] Status hanya boleh 'Belum' atau 'Selesai'!\n" << RESET;
                    waitForEnter();
                    continue;
                }
                for (const Task& task : tasks) {
                    if (task.status == status) {
                        filteredTasks.push_back(task);
                    }
                }
                clearScreen();
                displayHeader();
                std::cout << BLUE << "=== Hasil Pencarian ===\n\n" << RESET;
                displayTasks(filteredTasks);
            } else if (choice == 3) {
                std::string keyword;
                std::cout << "\nMasukkan kata kunci Nama Mata Kuliah: ";
                std::cin.sync();
                std::getline(std::cin, keyword);
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
                for (const Task& task : tasks) {
                    std::string courseName = task.course;
                    std::transform(courseName.begin(), courseName.end(), courseName.begin(), ::tolower);
                    if (courseName.find(keyword) != std::string::npos) {
                        filteredTasks.push_back(task);
                    }
                }
                clearScreen();
                displayHeader();
                std::cout << BLUE << "=== Hasil Pencarian ===\n\n" << RESET;
                displayTasks(filteredTasks);
            } else if (choice == 4) {
                std::cout << GREEN << "\n[*] Kembali ke menu tugas.\n" << RESET;
                waitForEnter();
                break;
            } else {
                std::cout << RED << "\n[!] Pilihan tidak valid!\n" << RESET;
                waitForEnter();
            }
        } while (true);
    }

    // Fungsi menampilkan notifikasi 
    void showDeadlineNotifications() {
        std::vector<Task> nearDeadlineTasks;
        for (const Task& task : tasks) {
            if (task.status == "Belum") {
                int daysLeft = calculateDaysDifference(task.deadline);
                if (daysLeft <= notificationDays && daysLeft >= 0) {
                    nearDeadlineTasks.push_back(task);
                }
            }
        }

        if (nearDeadlineTasks.empty()) return;

        clearScreen();
        displayHeader();
        std::cout << YELLOW << "=== Peringatan Tenggat Waktu ===\n\n" << RESET;
        const int noWidth = 5;
        const int nameWidth = 20;
        const int courseWidth = 25;
        const int deadlineWidth = 15;
        const int daysLeftWidth = 15;

        std::string separator = "+" + std::string(noWidth, '-') + "+" + std::string(nameWidth, '-') + "+" + 
                                std::string(courseWidth, '-') + "+" + std::string(deadlineWidth, '-') + "+" + 
                                std::string(daysLeftWidth, '-') + "+";

        std::cout << separator << "\n";
        std::cout << "|" << std::setw(noWidth) << std::left << " No " << "|" 
                  << std::setw(nameWidth) << " Nama Tugas " << "|" 
                  << std::setw(courseWidth) << " Nama Mata Kuliah " << "|" 
                  << std::setw(deadlineWidth) << " Tenggat " << "|" 
                  << std::setw(daysLeftWidth) << " Sisa Hari " << "|\n";
        std::cout << separator << "\n";

        for (size_t i = 0; i < nearDeadlineTasks.size(); ++i) {
            int daysLeft = calculateDaysDifference(nearDeadlineTasks[i].deadline);
            std::cout << "|" << std::setw(noWidth - 1) << std::left << (i + 1) << " |" 
                      << std::setw(nameWidth - 1) << (nearDeadlineTasks[i].name.length() > nameWidth - 2 ? nearDeadlineTasks[i].name.substr(0, nameWidth - 5) + "..." : nearDeadlineTasks[i].name) << " |" 
                      << std::setw(courseWidth - 1) << (nearDeadlineTasks[i].course.length() > courseWidth - 2 ? nearDeadlineTasks[i].course.substr(0, courseWidth - 5) + "..." : nearDeadlineTasks[i].course) << " |" 
                      << std::setw(deadlineWidth - 1) << nearDeadlineTasks[i].deadline << " |"
                      << std::setw(daysLeftWidth - 1) << daysLeft << " |";
            std::cout << "\n";
        }

        std::cout << separator << "\n";
        waitForEnter();
    }

    void startSession() {
        showDeadlineNotifications();
        clearScreen();
    }

    void setNotificationDays() {
        clearScreen();
        displayHeader();
        std::cout << BLUE << "=== Atur Batas Notifikasi Tenggat ===\n\n" << RESET;
        std::cout << "Batas notifikasi saat ini: " << notificationDays << " hari\n";
        std::cout << "Masukkan batas baru (minimal 1 hari): ";
        int newDays;
        if (!(std::cin >> newDays)) {
            std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            waitForEnter();
            return;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (newDays < 1) {
            std::cout << RED << "\n[!] Batas minimal adalah 1 hari!\n" << RESET;
        } else {
            notificationDays = newDays;
            saveNotificationDays();
            std::cout << GREEN << "\n[*] Batas notifikasi berhasil diubah ke " << notificationDays << " hari!\n" << RESET;
        }
        waitForEnter();
    }
};

// Fungsi untuk register pengguna
void registerUser() {
    clearScreen();
    displayHeader();
    std::cout << BLUE << "=== Registrasi Pengguna Baru ===\n\n" << RESET;
    
    std::string username, password;
    std::cout << "Masukkan username: ";
    std::cin.sync();
    std::getline(std::cin, username);

    std::ifstream checkFile("data/users.csv");
    std::string line;
    while (std::getline(checkFile, line)) {
        std::stringstream ss(line);
        std::string storedUsername;
        std::getline(ss, storedUsername, ',');
        if (storedUsername == username) {
            std::cout << RED << "\n[!] Username sudah digunakan! Silakan pilih username lain.\n" << RESET;
            checkFile.close();
            waitForEnter();
            return;
        }
    }
    checkFile.close();

    std::cout << "Masukkan password: ";
    std::cin.sync();
    std::getline(std::cin, password);

    showLoading();
    User newUser(username, password);
    newUser.saveToFile();
    logActivity("Pengguna '" + username + "' terdaftar");
    std::cout << GREEN << "\n[*] Registrasi pengguna baru: " << username << " berhasil! Silakan login.\n" << RESET;
    waitForEnter();
}

// Fungsi untuk login
TaskManager* loginUser() {
    clearScreen();
    displayHeader();
    std::cout << BLUE << "=== Login ke Aplikasi ===\n\n" << RESET;
    
    std::string username, password;
    std::cout << "Masukkan username: ";
    std::cin.sync();
    std::getline(std::cin, username);
    std::cout << "Masukkan password: ";
    std::cin.sync();
    std::getline(std::cin, password);

    showLoading();
    std::ifstream file("data/users.csv");
    if (!file.is_open()) {
        std::cout << RED << "\n[!] Gagal membuka file pengguna!\n" << RESET;
        waitForEnter();
        return nullptr;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string storedUsername, storedPassword;
        if (std::getline(ss, storedUsername, ',') && std::getline(ss, storedPassword, ',')) {
            User user(storedUsername, storedPassword);
            if (user.authenticate(username, password)) {
                file.close();
                logActivity("Pengguna '" + username + "' login");
                std::cout << GREEN << "\n[*] Login berhasil!\n" << RESET;
                waitForEnter();
                TaskManager* manager = new TaskManager(username, password);
                manager->startSession();
                return manager;
            }
        }
    }
    file.close();
    std::cout << RED << "\n[!] Login gagal! Username atau password salah.\n" << RESET;
    waitForEnter();
    return nullptr;
}

// Fungsi untuk lupa password
void forgotPassword() {
    clearScreen();
    displayHeader();
    std::cout << BLUE << "=== Lupa Password ===\n\n" << RESET;
    
    std::string username;
    std::cout << "Masukkan username: ";
    std::cin.sync();
    std::getline(std::cin, username);

    std::ifstream file("data/users.csv");
    if (!file.is_open()) {
        std::cout << RED << "\n[!] Gagal membuka file pengguna!\n" << RESET;
        waitForEnter();
        return;
    }

    std::string line;
    bool found = false;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string storedUsername, storedPassword;
        if (std::getline(ss, storedUsername, ',') && std::getline(ss, storedPassword, ',')) {
            if (storedUsername == username) {
                std::cout << GREEN << "\n[*] Password Anda: " << storedPassword << "\n" << RESET;
                found = true;
                break;
            }
        }
    }
    file.close();
    if (!found) {
        std::cout << RED << "\n[!] Username tidak ditemukan!\n" << RESET;
    }
    waitForEnter();
}

// Fungsi utama
int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    #ifdef _WIN32
        system("mkdir data");
    #else
        system("mkdir -p data");
    #endif

    int choice;
    TaskManager* currentUser = nullptr;

    while (true) {
        clearScreen();
        displayHeader();
        std::cout << CYAN << "===== Menu Utama =====" << RESET << "\n";
        std::cout << "| " << MAGENTA << BOLD << "1. Login           🖐" << RESET << " |\n";
        std::cout << "| " << MAGENTA << BOLD << "2. Register        📝" << RESET << " |\n";
        std::cout << "| " << MAGENTA << BOLD << "3. Lupa Password   🔑" << RESET << " |\n";
        std::cout << "| " << MAGENTA << BOLD << "4. Keluar          🚪" << RESET << " |\n";
        std::cout << CYAN << "=====================" << RESET << "\n";
        std::cout << "Pilih opsi [1-4]: ";
        if (!(std::cin >> choice)) {
            std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            waitForEnter();
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            currentUser = loginUser();
            if (currentUser) {
                while (true) {
                    currentUser->displayMenu();
                    if (!(std::cin >> choice)) {
                        std::cout << RED << "\n[!] Input tidak valid. Harus berupa angka.\n" << RESET;
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        waitForEnter();
                        continue;
                    }
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    if (choice == 1) {
                        currentUser->addTask();
                    } else if (choice == 2) {
                        currentUser->viewTasks();
                    } else if (choice == 3) {
                        currentUser->editTask();
                    } else if (choice == 4) {
                        currentUser->deleteTask();
                    } else if (choice == 5) {
                        currentUser->searchFilterTasks();
                    } else if (choice == 6) {
                        currentUser->setNotificationDays();
                    } else if (choice == 7) {
                        std::cout << GREEN << "\n[*] Berhasil keluar dari akun " << currentUser->getUsername() << ".\n" << RESET;
                        delete currentUser;
                        currentUser = nullptr;
                        waitForEnter();
                        clearScreen();
                        break;
                    } else {
                        std::cout << RED << "\n[!] Pilihan tidak valid!\n" << RESET;
                        waitForEnter();
                        clearScreen();
                        currentUser->displayMenu();
                    }
                }
            }
        } else if (choice == 2) {
            registerUser();
        } else if (choice == 3) {
            forgotPassword();
        } else if (choice == 4) {
            clearScreen();
            displayHeader();
            std::cout << GREEN << "\n[*] Terima kasih telah menggunakan Aplikasi Manajemen Tugas!\n" << RESET;
            break;
        } else {
            std::cout << RED << "\n[!] Pilihan tidak valid!\n" << RESET;
            waitForEnter();
            clearScreen();
            displayHeader();
        }
    }

    return 0; 
}