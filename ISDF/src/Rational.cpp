/* 
 * File:   Rational.cpp
 * Author: Wang Jinlin
 * 
 */

#include "Rational.h"

int gcd(int a, int b){
    if(a < b){
        int t = a;
        a = b;
        b = t;
    }
    while(b != 0){
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int lcm(int a, int b){
    return a * b / gcd(a, b);
}

/*
Rational::Rational() {
}

Rational::Rational(const Rational& orig) {
}
*/

Rational::Rational(int Top, int Bot)
        :top(Top), bot(Bot){
    if(bot == 0){
        top = 0;
        bot = 1;
    } else if(top == 0){
        bot = 1;
    } else {
        int g = gcd(top, bot);
        if(g != 1){
            top /= g;
            bot /= g;
        }
    }
}

Rational::~Rational() {
}

Rational operator*(const Rational &l, const Rational &r){
    return Rational(l.top * r.top, l.bot * r.bot);
}

Rational operator/(const Rational &l, const Rational &r){
    return Rational(l.top * r.bot, l.bot * r.top);
}
