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
};

unordered_map<int64_t, State> userStates;
unordered_map<int64_t, UserData> userData;






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





bool ninetwenty = true, nine = true, ninezerofive = true, nineten = true, ninefiveteen = true, ninetwentyfive = true, ninethirty = true, ninethirtyfive = true,
nineforty = true, ninefortyfive = true, ninefifty = true, ninefiftyfive = true, ten = true;


string messageText;


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
            bot.getApi().sendMessage(userId, "Введите удобное время посещения:");
            userStates[userId] = State::VISIT_TIME;
            break;
        case State::VISIT_TIME:
            userData[userId].visitTime = message->text;
            if (userData[userId].visitTime == "9:00") {
                if (nine) {
                    saveUserDataToFile(userData[userId]);
                    // Отправляем данные конкретному пользователю
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

        // Обработка специальной команды для отмены
        if (message->text == "/cancel") {
            userData[userId] = {};  // Очищаем данные пользователя
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


