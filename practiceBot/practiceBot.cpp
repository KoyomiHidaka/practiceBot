#include <iostream>
#include <iostream>
#include <tgbot/tgbot.h>
#include <unordered_map>
#pragma execution_character_set("utf-8")
// Замените recipientId на ID получателя сообщений
const int64_t recipientId = 1217311673;



using namespace TgBot;
using namespace std;

enum class State {
    START,
    FIRST_NAME,
    LAST_NAME,
    VISIT_TIME,
    CANCEL,
    DONE
};


// Структура хранящая данные пользователя

struct UserData {
    string firstName;
    string lastName;
    string visitTime;
    string userid;
    void ClearAll() {
        firstName.clear();
        lastName.clear();
        visitTime.clear();
    }
};

unordered_map<int64_t, State> userStates;
unordered_map<int64_t, UserData> userData;


// Структура хранящая параметры времени

struct TimeSlot
{
    string time;
    bool isAvailable;
};


// Массив хранящий время

TimeSlot timeSlots[] = {
        {"9:00", true}, {"9:10", true}, {"9:20", true}, {"9:30", true}, {"9:40", true},
        {"9:50", true}, {"10:00", true}, {"10:10", true}, {"10:20", true}, {"10:30", true},
        {"10:40", true}, {"10:50", true}, {"11:00", true}, {"11:10", true}, {"11:20", true},
        {"11:30", true}, {"11:40", true}, {"11:50", true}, {"12:00", true}, {"12:10", true},
        {"12:20", true}, {"12:30", true}, {"12:40", true}, {"12:50", true}, {"13:00", true}
};



// Блок кода сохраняющий данные в файл

void saveUserDataToFile(const UserData& userData) {
    ofstream outFile("visit_data.txt", ios::app);
    if (outFile.is_open()) {
        outFile << "UserId: " << userData.userid << '\n';
        outFile << "First Name: " << userData.firstName << "\n";
        outFile << "Last Name: " << userData.lastName << "\n";
        outFile << "Visit Time: " << userData.visitTime << "\n\n";
        outFile.close();
    }
    else {
        cerr << "Unable to open file for writing." << endl;
    }
}



bool first = true;
bool found = false;
string messageText;
string searchTime;
string timetable;


// Блок обработки машины состояний (FSM Context)

void handleUserState(const Bot& bot, int64_t userId, Message::Ptr message) {
    switch (userStates[userId]) {
        case State::START:
            bot.getApi().sendMessage(userId, "Введите имя:");
            userStates[userId] = State::FIRST_NAME;
            break;
        case State::FIRST_NAME:
            userData[userId].userid = to_string(userId);
            userData[userId].firstName = message->text;
            bot.getApi().sendMessage(userId, "Введите фамилию:");
            userStates[userId] = State::LAST_NAME;
            break;
        case State::LAST_NAME:
            userData[userId].lastName = message->text;
            for (const auto& slot : timeSlots) {
                if (slot.isAvailable) {
                    if (!first) {
                        timetable += ", ";
                    }
                    timetable += slot.time;
                    first = false;
                }
            }
            bot.getApi().sendMessage(userId, "Доступное время на сегодня: " + timetable); 
            bot.getApi().sendMessage(userId, "Введите удобное время посещения:");
            userStates[userId] = State::VISIT_TIME;
            break;
        case State::VISIT_TIME:
            userData[userId].visitTime = message->text;
            searchTime = userData[userId].visitTime;
            for (auto& slot : timeSlots) {
                if (!slot.isAvailable && slot.time == searchTime) {
                    slot.isAvailable = false;
                    found = true;
                    break;
                }
            }
            if (found)
            {
                bot.getApi().sendMessage(userId, "Время " + searchTime + " уже занято, выберите другое");
                userStates[userId] = State::LAST_NAME;
            }
            if (!found) {
                saveUserDataToFile(userData[userId]);
                messageText = "Данные для отправки:\n";
                messageText += "Имя: " + userData[userId].firstName + "\n";
                messageText += "Фамилия: " + userData[userId].lastName + "\n";
                messageText += "Время посещения: " + userData[userId].visitTime + "\n";
                messageText += "UserId: " + to_string(userId);
                bot.getApi().sendMessage(recipientId, messageText);
                bot.getApi().sendMessage(userId, "Ваши данные отправлены.");
                userStates[userId] = State::DONE;
                break;
            }
        case State::DONE:
            bot.getApi().sendMessage(userId, "Вы уже ввели свои данные.");
            break;
    }
}



int main()
{
    TgBot::Bot bot("7195793439:AAG7WjTsl5rhwud07JinMVGdGH9FORCbjnM");


    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        int64_t userId = message->chat->id;

        if (userStates.find(userId) == userStates.end()) {
            userStates[userId] = State::START;
        }
        // Блок обработки специальной команды для отмены
        if (message->text == "/cancel") {
            string cancelledTime = userData[userId].visitTime;
            string canceledName = userData[userId].firstName;
            string canceledLastname = userData[userId].lastName;
            userData[userId].ClearAll();
            for (auto& slot: timeSlots)
            {
                if (slot.time == cancelledTime) {
                    slot.isAvailable = true;
                    break;
                }
            }
            userStates[userId] = State::START;
            bot.getApi().sendMessage(userId, "Бронирование отменено. Начните заново.");
            string cancelMessage = "Пользователь " + to_string(userId)+ " (" + canceledName + ", " + canceledLastname + ")" + " отменил бронирование.";
            bot.getApi().sendMessage(recipientId, cancelMessage);
            ofstream outFile("visit_data.txt", ios::app);
            if (outFile.is_open())
            {
                outFile << "The user " + to_string(userId) + " has cancelled the reservation." << '\n';
                outFile.close();
            }
            else {
                cerr << "Unable to open file for writing." << endl;
            }
            
        }
        else {
            handleUserState(bot, userId, message);
        }
        });
    
// Блок запуска бота

    try {
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            cout << "Long poll started^^" << endl;
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        cout << "error: " << e.what() << endl;
    }

    return 0;

    
}


