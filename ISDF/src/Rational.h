/* 
 * File:   Rational.h
 * Author: Wang Jinlin
 *
 */

#ifndef RATIONAL_H
#define	RATIONAL_H

int gcd(int a, int b);
int lcm(int a, int b);

class Rational {
public:
    Rational(){}
    Rational(const Rational& orig)
            :top(orig.top), bot(orig.bot){}
    Rational(int Top, int Bot);
            //:top(Top), bot(Bot){}
    virtual ~Rational();

    Rational& operator=(const Rational &r){
        top = r.top;
        bot = r.bot;
        return *this;
    }
    bool operator==(const Rational &r) const {
        return (top == r.top && bot == r.bot)
                || (top == -r.top && bot == -r.bot);
    }
    bool operator!=(const Rational &r) const {
        return !(*this == r);
    }
    int getTop() const {
        return top;
    }
    int getBop() const {
        return bot;
    }

    friend Rational operator*(const Rational &l, const Rational &r);
    friend Rational operator/(const Rational &l, const Rational &r);
private:
    int top, bot;

};

#endif	/* RATIONAL_H */

