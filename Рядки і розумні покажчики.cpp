#include <iostream>
#include <utility> // для std::swap

using namespace std;

// Базовый класс для тестирования
class TestClass {
private:
    string name;
public:
    TestClass(const string& n) : name(n) {
        cout << "Создан объект: " << name << endl;
    }
    
    ~TestClass() {
        cout << "Удален объект: " << name << endl;
    }
    
    void greet() const {
        cout << "Привет от " << name << "!" << endl;
    }
};

// Аналог std::unique_ptr - единоличный владелец
template<typename T>
class UniquePtr {
private:
    T* ptr;

public:
    // Конструктор
    explicit UniquePtr(T* p = nullptr) : ptr(p) {}
    
    // Запрещаем копирование
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
    
    // Разрешаем перемещение
    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    
    // Деструктор
    ~UniquePtr() {
        delete ptr;
    }
    
    // Операторы доступа
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }
    
    // Освобождение владения
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }
    
    // Сброс указателя
    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }
    
    // Проверка на пустоту
    explicit operator bool() const { return ptr != nullptr; }
};

// Аналог std::shared_ptr - разделяемое владение
template<typename T>
class SharedPtr {
private:
    T* ptr;
    int* count; // счетчик ссылок

    void cleanup() {
        if (count && --(*count) == 0) {
            delete ptr;
            delete count;
        }
    }

public:
    // Конструкторы
    explicit SharedPtr(T* p = nullptr) : ptr(p), count(p ? new int(1) : nullptr) {}
    
    // Копирование
    SharedPtr(const SharedPtr& other) : ptr(other.ptr), count(other.count) {
        if (count) {
            ++(*count);
        }
    }
    
    // Присваивание
    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            cleanup();
            ptr = other.ptr;
            count = other.count;
            if (count) {
                ++(*count);
            }
        }
        return *this;
    }
    
    // Перемещение
    SharedPtr(SharedPtr&& other) noexcept : ptr(other.ptr), count(other.count) {
        other.ptr = nullptr;
        other.count = nullptr;
    }
    
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            cleanup();
            ptr = other.ptr;
            count = other.count;
            other.ptr = nullptr;
            other.count = nullptr;
        }
        return *this;
    }
    
    // Деструктор
    ~SharedPtr() {
        cleanup();
    }
    
    // Операторы доступа
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }
    
    // Получение счетчика ссылок
    int use_count() const { return count ? *count : 0; }
    
    // Проверка на пустоту
    explicit operator bool() const { return ptr != nullptr; }
};

// Функции для тестирования
void testUniquePtr() {
    cout << "\n=== ТЕСТ UNIQUE_PTR ===" << endl;
    
    // Создание unique_ptr
    UniquePtr<TestClass> u1(new TestClass("Unique Object 1"));
    u1->greet();
    
    // Перемещение владения
    {
        UniquePtr<TestClass> u2 = std::move(u1);
        if (!u1) {
            cout << "u1 теперь пустой" << endl;
        }
        u2->greet();
    } // u2 уничтожается здесь
    
    cout << "После блока с u2" << endl;
    
    // Создание нового объекта
    UniquePtr<TestClass> u3(new TestClass("Unique Object 2"));
    u3->greet();
}

void testSharedPtr() {
    cout << "\n=== ТЕСТ SHARED_PTR ===" << endl;
    
    // Создание shared_ptr
    SharedPtr<TestClass> s1(new TestClass("Shared Object 1"));
    cout << "s1 use_count: " << s1.use_count() << endl;
    s1->greet();
    
    // Копирование - увеличивает счетчик
    {
        SharedPtr<TestClass> s2 = s1;
        cout << "После копирования - s1 use_count: " << s1.use_count() << endl;
        cout << "После копирования - s2 use_count: " << s2.use_count() << endl;
        
        SharedPtr<TestClass> s3 = s1;
        cout << "После создания s3 - use_count: " << s1.use_count() << endl;
        
        s2->greet();
        s3->greet();
    } // s2 и s3 уничтожаются здесь
    
    cout << "После блока - s1 use_count: " << s1.use_count() << endl;
    s1->greet();
    
    // Новый независимый shared_ptr
    SharedPtr<TestClass> s4(new TestClass("Shared Object 2"));
    cout << "s4 use_count: " << s4.use_count() << endl;
}

void testBasicTypes() {
    cout << "\n=== ТЕСТ С БАЗОВЫМИ ТИПАМИ ===" << endl;
    
    // Тест с int для UniquePtr
    UniquePtr<int> intPtr(new int(42));
    cout << "Значение int: " << *intPtr << endl;
    *intPtr = 100;
    cout << "Измененное значение: " << *intPtr << endl;
    
    // Тест с int для SharedPtr
    SharedPtr<int> sharedInt(new int(500));
    cout << "Shared int: " << *sharedInt << ", счетчик: " << sharedInt.use_count() << endl;
    
    {
        SharedPtr<int> sharedInt2 = sharedInt;
        cout << "После копирования - счетчик: " << sharedInt.use_count() << endl;
        *sharedInt2 = 999;
        cout << "Через sharedInt: " << *sharedInt << ", через sharedInt2: " << *sharedInt2 << endl;
    }
    
    cout << "После блока - счетчик: " << sharedInt.use_count() << ", значение: " << *sharedInt << endl;
}

int main() {
    cout << "ДЕМОНСТРАЦИЯ УМНЫХ УКАЗАТЕЛЕЙ" << endl;
    
    testUniquePtr();
    testSharedPtr();
    testBasicTypes();
    
    cout << "\n=== ПРОГРАММА ЗАВЕРШЕНА ===" << endl;
    return 0;
}