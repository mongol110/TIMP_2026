#pragma once

// Классическая реализация Singleton (учебная, с ручным управлением памятью).
class SingletonClassic
{
private:
    static SingletonClassic* p_instance;
    SingletonClassic() {}
    SingletonClassic(const SingletonClassic&);
    SingletonClassic& operator=(SingletonClassic&);
public:
    static SingletonClassic* getInstance() {
        if (!p_instance) p_instance = new SingletonClassic();
        return p_instance;
    }
};
