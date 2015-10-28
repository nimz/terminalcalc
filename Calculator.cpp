#include <iostream>
#include <vector>
#include <math.h>

using namespace std; // small, standalone project - safe to use

enum LastOp { INFIX, LPAREN, RPAREN, OTHER };

bool isNumeric(char c) {
    return c >= '0' && c <= '9';
}

// returns whether the character is considered valid
// by the evaluator
bool isValid (char c) {
    return isNumeric(c) || c == '(' || c == ')'
    || c == '+' || c == '-' || c == '*'
    || c == '/' || c == '^' || c == '.'
    || c == ' ';
}

// operator precedence
short toPrecedence(char c) {
    if (c == '^')
        return 4;
    if (c == '/' || c == '*')
        return 3;
    if (c == '+' || c == '-')
        return 2;
    return -1;
}

// does this character correspond to a right-associative operator?
short isRightAssoc(char c) {
    return c == '^';
}

// token: either operator or an operand
class Token {
public:
    bool isOperator;
    string str;
};

// Operator: a token containing an arithmetic operator
class Operator: public Token {
public:
    char type;
    short precedence;
    bool rightAssociative;
    Operator();
    Operator(string s);
};

Operator::Operator() {}

Operator::Operator(string s) {
    isOperator = true;
    type = s[0];
    precedence = toPrecedence(type);
    rightAssociative = isRightAssoc(type);
    str = s;
}

// Operand: a token containing a double-precision real number
class Operand: public Token {
public:
    double value;
    Operand(string s);
};

Operand::Operand(string s) {
    isOperator = false;
    value = stod(s);
    str = s;
}

// Linked list nodes, each containing a pointer to a token
class ListNode {
public:
    ListNode* next;
    Token *data;
    ListNode(Token *d);
};

ListNode::ListNode(Token *d) {
    data = d;
    next = NULL;
}

// basic, non-templated singly linked list to hold numeric or operator tokens
class LinkedList {
public:
    ListNode* head;
    ListNode* tail;
    void push(Token *t);
    Token pop();
    void printAllNodes();
    void reset();
};

void LinkedList::push(Token *t) {
    ListNode* nextNode = new ListNode(t);
    if (head == NULL) {
        head = nextNode;
        tail = head;
        return;
    }
    tail->next = nextNode;
    tail = tail->next;
}

Token LinkedList::pop() {
    Token returnTok = *(head->data);
    head = head->next;
    return returnTok;
}

void LinkedList::printAllNodes() {
    ListNode* v = head;
    while (v) {
        cout << v->data->str << " ";
        v = v->next;
    }
    cout << endl;
}

void LinkedList::reset() {
    head = NULL;
    tail = NULL;
}

void processInfixOperator (Operator* op, vector<Operator*> &opStack,
                           int &stackIndex, LinkedList &outQ) {
    while (stackIndex >= 0 &&
           op->precedence <= opStack[stackIndex]->precedence
           - opStack[stackIndex]->rightAssociative) {
        outQ.push(opStack[stackIndex--]);
    }
    opStack[++stackIndex] = op;
}

const string exitStr = "exit";
// returns true if this string contains the word "exit" (even if it contains
// other characters, will still be treated as an exit signal for simplicity)
bool isExit (string s, int index) {
    if (index > s.length() - 4) return false;
    for (int i = index; i < index + 4; i++) {
        // test if the characters are equal, ignoring case
        if (s[i] != exitStr[i-index] && s[i]+32 != exitStr[i-index])
            return false;
    }
    return true;
}

bool validate(string s) {
    for (int i = 0; i < s.length(); i++) {
        if (!isValid(s[i]) || (i > 0 && s[i-1] == '(' && s[i] == ')')) {
            if (isExit(s, i))
                exit(0);
            return false;
        }
    }
    return true;
}

bool isStartOfNumber(char c, LastOp l) {
    return isNumeric(c) || ((l == INFIX || l == LPAREN) && c == '-') || c == '.';
}

int validateNumber(string s, int i) {
    bool seenPeriod = false;
    while (i < s.length() && (isNumeric(s[i]) || s[i] == '.')) {
        if (s[i] == '.') {
            if (seenPeriod) {
                cout << "Error: Too many periods." << endl;
                return -1;
            }
            seenPeriod = true;
        }
        i++;
    }
    return i;
}

LinkedList* shuntingYard (string s) {
    LinkedList* oq = new LinkedList;
    LinkedList& outQ = *oq;
    vector<Operator*> opStack(s.length());
    int stackIndex = -1;
    
    LastOp last_op = LPAREN;
    int i = 0;
    while (i < s.length()) {
        while (i < s.length() && s[i] == ' ') {
            i++;
        }
        if (i >= s.length())
            break;
        if (isStartOfNumber(s[i], last_op))
        {
            int startIndex = i++;
            i = validateNumber(s, i);
            if (i == -1) {
                cout << "Error: Invalid input." << endl;
                return NULL;
            }
            Operand* op = new Operand(s.substr(startIndex, i - startIndex));
            if (last_op == RPAREN) { // (expr)y = (expr) * y
                Operator* m = new Operator ("*");
                processInfixOperator(m, opStack, stackIndex, outQ);
            }
            outQ.push(op);
            last_op = OTHER;
        }
        else {
            // the operator is the next character
            Operator* op = new Operator(s.substr(i++, 1));
            Operator o = *op;
            if (o.type == '(') {
                // x(expr) = x * (expr); (expr1)(expr2) = (expr1) * (expr2)
                if (last_op != INFIX && last_op != LPAREN) {
                    Operator* m = new Operator ("*"); // for ex: 3(4+2)=3*(4+2)
                    processInfixOperator(m, opStack, stackIndex, outQ);
                    last_op = INFIX;
                }
                opStack[++stackIndex] = op;
                last_op = LPAREN;
            }
            else if (o.type == ')') {
                while (stackIndex >= 0 && opStack[stackIndex]->type != '(') {
                    outQ.push(opStack[stackIndex--]);
                }
                if (stackIndex == -1) {
                    cout << "Error: Mismatched parentheses." << endl;
                    return NULL;
                }
                stackIndex--;
                last_op = RPAREN;
            }
            else {
                processInfixOperator(op, opStack, stackIndex, outQ);
                last_op = INFIX;
            }
        }
    }
    while (stackIndex >= 0) {
        char t = opStack[stackIndex]->type;
        if (t == ')' || t == ')') {
            cout << "Error: Mismatched parentheses." << endl;
            return NULL;
        }
        outQ.push(opStack[stackIndex--]);
    }
    outQ = *oq;
    return oq;
}

struct doubleWithStatus { double d; bool s; };
doubleWithStatus evaluateRPN(LinkedList outQ, size_t slen) {
    doubleWithStatus success; success.s = true;
    doubleWithStatus failure; failure.s = false;
    vector<double> numStack(slen);
    int stackIndex = -1;
    ListNode *node = outQ.head;
    while (node) {
        if (!node->data->isOperator) {
            Operand* x = static_cast<Operand*>(node->data);
            numStack[++stackIndex] = x->value;
            node = node->next;
        }
        else {
            if (stackIndex < 1) {
                cout << "Error: Too many operators." << endl;
                return failure;
            }
            double a = numStack[stackIndex--], b = numStack[stackIndex--];
            Operator* y = static_cast<Operator*>(node->data);
            switch (y->type) {
                case '+':
                    numStack[++stackIndex] = b+a;
                    break;
                case '-':
                    numStack[++stackIndex] = b-a;
                    break;
                case '*':
                    numStack[++stackIndex] = b*a;
                    break;
                case '/':
                    if (a == 0) {
                        cout << "Division by zero." << endl;
                        return failure;
                    }
                    numStack[++stackIndex] = b/a;
                    break;
                case '^':
                    if (b == 0 && a < 0) {
                        cout << "Division by zero." << endl;
                        return failure;
                    }
                    else if (b < 0) {
                        double intpart;
                        if (modf(a, &intpart) != 0.0) {
                            cout << "Exponentiation of negative base to "
                            << "non-integral power." << endl;
                            return failure;
                        }
                    }
                    numStack[++stackIndex] = pow(b, a);
                    break;
            }
            node = node->next;
        }
    }
    if (stackIndex != 0) {
        cout << "Error: Not enough operators." << endl;
        return failure;
    }
    success.d = numStack[0];
    return success;
}

int main () {
    cout << "This program takes in an arithmetic expression in infix " <<
    "notation and evaluates it. Operations supported are addition, " <<
    "subtraction, multiplication, division, and exponentiation, " <<
    "along with parenthetical grouping of operations. " <<
    "Please do not enter any non-numeric characters besides " <<
    "decimal points, spaces and the characters +-*/^() " <<
    "corresponding to operators and parentheses. " <<
    "Also note that x ^ y ^ z is interpreted as x ^ (y ^ z) " <<
    "and that exponentiation of negative numbers to non-integral " <<
    "powers is not allowed because fractions are treated as decimals." <<
    endl << "Enjoy!" << endl;
    
    while (true) {
        string s = "";
        int attemptCount = 0;
        while (s.length() == 0) {
            if (attemptCount++ == 0)
                cout << "Please enter the expression you wish to evaluate, " <<
                "or type \"exit\" to exit." << endl;
            else
                cout << "Please enter a valid input:" << endl;
            getline(cin, s);
            if (!validate(s))
                s = "";
        }
        
        // the following block of code converts the input to reverse Polish
        // notation using the shunting-yard algorithm, and outputs the tokens
        // to a queue
        LinkedList* oq = shuntingYard(s);
        if (oq == NULL)
            continue;
        LinkedList outQ = *oq;
        
        doubleWithStatus dstat = evaluateRPN(outQ, s.length());
        if (dstat.s) {
            cout << "The expression's value is " << dstat.d << endl;
        }
    }
    return 0;
}