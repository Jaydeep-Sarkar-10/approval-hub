#include "crow_all.h"
#include "asio.hpp"
#include <string>
using namespace std;

// ─────────────────────────────────────────
// CONSTANTS
// ─────────────────────────────────────────
const int MAX_USERS = 10;
const int MAX_DOCS = 20;
const int MAX_COMMENTS = 5;

// ─────────────────────────────────────────
// STRUCTS
// ─────────────────────────────────────────
class User {
public:
    string name, email, password, role;
    bool active;
};

class Comment {
public:
    string author, text;
    bool used;
};

class Document {
public:
    int id;
    string title, description, filename, uploaded_by, status;
    Comment comments[MAX_COMMENTS];
    int comment_count;
    bool active;
};

// ─────────────────────────────────────────
// GLOBAL DATA
// ─────────────────────────────────────────
User users[MAX_USERS];
Document docs[MAX_DOCS];

int user_count = 0;
int doc_count = 0;
int doc_id_counter = 1;

// ─────────────────────────────────────────
// SEED DATA
// ─────────────────────────────────────────
void seedUsers() {
    users[0] = {"Admin User","admin@portal.com","admin123","admin",true};
    users[1] = {"Acme Corp","client@acme.com","client123","client",true};
    user_count = 2;
}

void seedDocuments() {
    docs[0].id = doc_id_counter++;
    docs[0].title = "Q1 Campaign Proposal";
    docs[0].description = "Marketing campaign plan";
    docs[0].filename = "q1.pdf";
    docs[0].uploaded_by = "Admin User";
    docs[0].status = "Pending";
    docs[0].comment_count = 0;
    docs[0].active = true;

    doc_count = 1;
}

// ─────────────────────────────────────────
// HELPERS
// ─────────────────────────────────────────
int findDocById(int id) {
    for (int i = 0; i < MAX_DOCS; i++) {
        if (docs[i].active && docs[i].id == id)
            return i;
    }
    return -1;
}

// ─────────────────────────────────────────
// MAIN SERVER
// ─────────────────────────────────────────
int main() {

    seedUsers();
    seedDocuments();

    crow::App<crow::CORSHandler> app;

    app.get_middleware<crow::CORSHandler>()
        .global()
        .origin("*")
        .methods("GET"_method, "POST"_method);

    // ───────────────── LOGIN ─────────────────
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);

        string email = body["email"].s();
        string password = body["password"].s();

        for(int i=0;i<MAX_USERS;i++){
            if(users[i].active &&
               users[i].email == email &&
               users[i].password == password){
                crow::json::wvalue res;
                res["message"] = "Login success";
                res["name"] = users[i].name;
                res["role"] = users[i].role;
                return crow::response(res);
            }
        }
        return crow::response(401, "Invalid credentials");
    });

    // ─────────────── GET ALL DOCUMENTS ───────────────
    CROW_ROUTE(app, "/documents")
    ([](){
        crow::json::wvalue result;

        int index = 0;
        for (int i = 0; i < MAX_DOCS; i++) {
            if (docs[i].active) {
                result[index]["id"] = docs[i].id;
                result[index]["title"] = docs[i].title;
                result[index]["status"] = docs[i].status;
                result[index]["uploaded_by"] = docs[i].uploaded_by;
                index++;
            }
        }
        return result;
    });

    // ─────────────── GET DOCUMENT BY ID ───────────────
    CROW_ROUTE(app, "/document/<int>")
    ([](int id){
        int idx = findDocById(id);
        if(idx == -1) return crow::response(404, "Not found");

        crow::json::wvalue res;
        res["id"] = docs[idx].id;
        res["title"] = docs[idx].title;
        res["description"] = docs[idx].description;
        res["status"] = docs[idx].status;

        for(int i=0;i<MAX_COMMENTS;i++){
            if(docs[idx].comments[i].used){
                res["comments"][i]["author"] = docs[idx].comments[i].author;
                res["comments"][i]["text"] = docs[idx].comments[i].text;
            }
        }

        return crow::response(res);
    });

    // ─────────────── ADD COMMENT ───────────────
    CROW_ROUTE(app, "/comment").methods("POST"_method)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);

        int id = body["id"].i();
        string text = body["text"].s();
        string author = body["author"].s();

        int idx = findDocById(id);
        if(idx == -1) return crow::response(404, "Doc not found");

        for(int i=0;i<MAX_COMMENTS;i++){
            if(!docs[idx].comments[i].used){
                docs[idx].comments[i] = {author, text, true};
                docs[idx].comment_count++;
                return crow::response("Comment added");
            }
        }

        return crow::response(400, "Comment limit reached");
    });

    // ─────────────── CHANGE STATUS ───────────────
    CROW_ROUTE(app, "/status").methods("POST"_method)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);

        int id = body["id"].i();
        string status = body["status"].s();

        int idx = findDocById(id);
        if(idx == -1) return crow::response(404, "Not found");

        docs[idx].status = status;
        return crow::response("Status updated");
    });

    // ─────────────── UPLOAD DOCUMENT ───────────────
    CROW_ROUTE(app, "/upload").methods("POST"_method)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);

        if(doc_count >= MAX_DOCS)
            return crow::response(400, "Limit reached");

        for(int i=0;i<MAX_DOCS;i++){
            if(!docs[i].active){
                docs[i].id = doc_id_counter++;
                docs[i].title = body["title"].s();
                docs[i].description = body["description"].s();
                docs[i].filename = body["filename"].s();
                docs[i].uploaded_by = body["author"].s();
                docs[i].status = "Pending";
                docs[i].comment_count = 0;
                docs[i].active = true;
                doc_count++;
                return crow::response("Uploaded");
            }
        }
        return crow::response(500);
    });

    // ─────────────── DELETE DOCUMENT ───────────────
    CROW_ROUTE(app, "/delete/<int>")
    ([](int id){
        int idx = findDocById(id);
        if(idx == -1) return crow::response(404);

        docs[idx].active = false;
        doc_count--;
        return crow::response("Deleted");
    });

    CROW_ROUTE(app, "/")
    ([](){
        return "C++ Backend is running ✅";
    });

    app.bindaddr("127.0.0.1").port(5000).multithreaded().run();
}
