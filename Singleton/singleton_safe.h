#pragma once

// Безопасные реализации: через Destroyer и через Meyers (C++11).
class SingletonSafe;

class SingletonDestroyer
{
private:
    SingletonSafe* p_instance = nullptr;
public:
    ~SingletonDestroyer();
    void initialize(SingletonSafe* p) { p_instance = p; }
};

class SingletonSafe
{
private:
    static SingletonSafe* p_instance;
    static SingletonDestroyer destroyer;
    SingletonSafe() {}
    SingletonSafe(const SingletonSafe&);
    SingletonSafe& operator=(SingletonSafe&);
    ~SingletonSafe() {}
    friend class SingletonDestroyer;
public:
    static SingletonSafe* getInstance() {
        if (!p_instance) {
            p_instance = new SingletonSafe();
            destroyer.initialize(p_instance);
        }
        return p_instance;
    }
};

class SingletonModern
{
public:
    static SingletonModern& getInstance() {
        static SingletonModern instance;
        return instance;
    }
    SingletonModern(const SingletonModern&) = delete;
    SingletonModern& operator=(const SingletonModern&) = delete;
private:
    SingletonModern() {}
};
