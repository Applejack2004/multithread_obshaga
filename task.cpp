#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <random>
#include <ctime>
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 8); // 1-8 seconds
std::uniform_int_distribution<> dis2(0, 1);//veroyatnostb vihoda i zaxoda
std::uniform_int_distribution<> countDis(1, 5); // 1-5 people
int woman_in_bath = 0;
int man_in_bath = 0;
// Enum for the three states of the bathroom
enum BathroomState { FREE, OCCUPIED_BY_WOMEN, OCCUPIED_BY_MEN };

// Mutex and condition variable for synchronization
std::mutex mtx;
std::condition_variable cv;

// Current state of the bathroom
BathroomState state = BathroomState::FREE;

std::string getStateString(BathroomState state);

// Function to get current time as a string
char* getCurrentTime() {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return asctime(timeinfo);
}

// Procedure for a woman who wants to enter
void womanEnters() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return state == BathroomState::FREE || state == BathroomState::OCCUPIED_BY_WOMEN; });
    state = BathroomState::OCCUPIED_BY_WOMEN;
    std::cout << getCurrentTime() << "Woman enters. State: " << getStateString(state) << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
}

// Procedure for a man who wants to enter
void manEnters() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return state == BathroomState::FREE || state == BathroomState::OCCUPIED_BY_MEN; });
    state = BathroomState::OCCUPIED_BY_MEN;
    std::cout << getCurrentTime() << "Man enters. State: " << getStateString(state) << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
}

// Procedure for a woman who wants to exit
void womanExits() {
    std::lock_guard<std::mutex> lock(mtx);
    if (state == BathroomState::OCCUPIED_BY_WOMEN && woman_in_bath == 1) {
        state = BathroomState::FREE;
        std::cout << getCurrentTime() << "Woman exits. State: " << getStateString(state) << std::endl;
        cv.notify_all();
    }
    else
    {
        std::cout << getCurrentTime() << "Woman exits. State: " << getStateString(state) << std::endl;
    }


}

// Procedure for a man who wants to exit
void manExits() {
    std::lock_guard<std::mutex> lock(mtx);
    if (state == BathroomState::OCCUPIED_BY_MEN && man_in_bath == 1) {
        state = BathroomState::FREE;
        std::cout << getCurrentTime() << "Man exits. State: " << getStateString(state) << std::endl;
        cv.notify_all();

    }
    else
    {
        std::cout << getCurrentTime() << "Man exits. State: " << getStateString(state) << std::endl;
    }


}

// Helper function to get a string representation of the bathroom state
std::string getStateString(BathroomState state) {
    switch (state) {
    case BathroomState::FREE:
        return "Free";
    case BathroomState::OCCUPIED_BY_WOMEN:
        return "Occupied by women";
    case BathroomState::OCCUPIED_BY_MEN:
        return "Occupied by men";
    }
}

int main() {


    bool womanTurn = true;
    while (true)
    {
        std::thread t1([&] {

            if (womanTurn) {
                int womancount = countDis(gen);

                int woman_was_in_bath = 0;

                // Женщины заходят в душевую или выходят
                while (woman_was_in_bath < womancount && woman_in_bath <= womancount) {
                    if (getStateString(state) == "Free") {
                        std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
                        womanEnters();
                        woman_in_bath++;
                    }
                    else {
                        int tmp;
                        if (woman_in_bath == womancount || (woman_in_bath + woman_was_in_bath) == womancount) {
                            tmp = 0;
                        }
                        else {
                            tmp = dis2(gen);
                        }

                        if (tmp == 1) {
                            std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
                            womanEnters();
                            woman_in_bath++;
                        }
                        else {
                            std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
                            womanExits();
                            woman_in_bath--;
                            woman_was_in_bath++;
                        }
                    }
                }

                // Все женщины, которые зашли, выходят из душевой
               /* while (woman_in_bath > 0) {
                    womanExits();
                    woman_in_bath--;
                }*/

                womanTurn = false;
            }

            });

        std::thread t2([&] {

            if (!womanTurn) {
                int mancount = countDis(gen);
                int man_was_in_bath = 0;

                // Мужчины заходят в душевую или выходят
                while (man_was_in_bath < mancount && man_in_bath <= mancount) {
                    if (getStateString(state) == "Free") {
                        std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
                        manEnters();
                        //
                        man_in_bath++;
                    }
                    else {
                        int tmp;
                        if (man_in_bath == mancount || (man_in_bath + man_was_in_bath) == mancount) {
                            tmp = 0;
                        }
                        else {
                            tmp = dis2(gen);
                        }

                        if (tmp == 1) {
                            std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
                            manEnters();
                            man_in_bath++;
                        }
                        else {
                            std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));
                            manExits();
                            man_in_bath--;
                            man_was_in_bath++;
                        }
                    }
                }

                // Все мужчины, которые зашли, выходят из душевой
               /* while (man_in_bath > 0) {
                    manExits();
                    man_in_bath--;
                }*/

                womanTurn = true;
            }

            });

        // Stop the simulation
        t1.join();
        t2.join();
    }


    return 0;
}