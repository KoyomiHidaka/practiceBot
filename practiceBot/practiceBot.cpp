#include <iostream>
#include <iostream>
#include <tgbot/tgbot.h>
#include <unordered_map>
#pragma execution_character_set("utf-8")
// Замените YOUR_RECIPIENT_ID на реальный ID получателя сообщений
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

struct UserData {
    string firstName;
    string lastName;
    string visitTime;
    string userid;
    void ClearNames() {
        firstName.clear();
        lastName.clear();
        visitTime.clear();
    }
};

unordered_map<int64_t, State> userStates;
unordered_map<int64_t, UserData> userData;

struct TimeSlot
{
    string time;
    bool isAvailable;
};

TimeSlot timeSlots[] = {
        {"9:00", true}, {"9:05", true}, {"9:10", true}, {"9:15", true}, {"9:20", true},
        {"9:25", true}, {"9:30", true}, {"9:35", true}, {"9:40", true}, {"9:45", true},
        {"9:50", true}, {"9:55", true}, {"10:00", true}
};

bool found = false;




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
string messageText;
string searchTime;
string timetable;

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
            //bot.getApi().sendMessage(userId, "Доступное время на сегодня: ");
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
            //bot.getApi().sendMessage(userId, timetable + " Доступное время на сегодня. ");
            bot.getApi().sendMessage(userId, "Введите удобное время посещения:");
            userStates[userId] = State::VISIT_TIME;
            break;
        case State::VISIT_TIME:
            userData[userId].visitTime = message->text;
            searchTime = userData[userId].visitTime;
           //for (auto& slot : timeSlots) {
           //    if (slot.time == searchTime) {
           //        slot.isAvailable = false;
           //        found = true;
           //        break;
           //    }
           //}
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
            //if (userData[userId].visitTime == "9:00")
            //{
            //    timeSlots[0].isAvailable = false;
            //}
            //saveUserDataToFile(userData[userId]);
            // Отправляем данные конкретному пользователю
            //messageText = "Данные для отправки:\n";
            //messageText += "Имя: " + userData[userId].firstName + "\n";
            //messageText += "Фамилия: " + userData[userId].lastName + "\n";
            //messageText += "Время посещения: " + userData[userId].visitTime + "\n";
            //messageText += "UserId: " + to_string(userId);
            //bot.getApi().sendMessage(recipientId, messageText);
            //bot.getApi().sendMessage(userId, "Ваши данные отправлены.");
            //userStates[userId] = State::DONE;
            //break;
            
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

        // Обработка специальной команды для отмены
        if (message->text == "/cancel") {
            string cancelledTime = userData[userId].visitTime;
            userData[userId].ClearNames();
            for (auto& slot: timeSlots)
            {
                if (slot.time == cancelledTime) {
                    slot.isAvailable = true;
                    break;
                }
            }
            cout << cancelledTime;
            //userData[userId] = {};  // Очищаем данные пользователя
            userStates[userId] = State::START;
            bot.getApi().sendMessage(userId, "Бронирование отменено. Начните заново.");
            string cancelMessage = "Пользователь " + to_string(userId) + " отменил бронирование.";
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


