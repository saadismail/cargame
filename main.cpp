#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>

using namespace sf;

// Number of objects of obstacle cars to be spawned
#define noOfobs 200

// Width & Height of graphical window
int width = 800;
int height = 600;
sf::Color gray(105, 105, 105);
// Positions of tracks
int in[6] = { 0,100,200,300,400,500 };
// Velocity
int velocity = 3;
// Width of tracks & segments
int trackW = 90, segW = 12;
// Array to keep a track of which cars are on which track (0 if now spawned)
int obsP[noOfobs];
sf::Clock c;
bool accelerate = false;
int lastcartrack=0;
bool started = false;
int score;

int check = 60;
int temp = 0;
int ptempx, ptempy;
bool collision = false, collisionNow = false;

// Initialize all obstacle cars' positions to 0
void init() {
	for (int i = 0; i<noOfobs; i++) {
		obsP[i] = 0;
	}
}

// Returns int a random number from low to up inclusive
int my_rand(int low, int up)
{
	static std::mt19937 rng(std::time(nullptr));
	static std::uniform_int_distribution<int> distrib(low, up);
	return distrib(rng);
}

void moveSeg(int &y) {
	y += velocity;
	if (y>height) {
		y -= height;
	}
}

void drawQuad(RenderWindow &w, Color c, int x1, int y1, int x2, int y2)
{
	ConvexShape shape(4);
	shape.setFillColor(c);
	shape.setPoint(0, Vector2f(x1, y1));
	shape.setPoint(1, Vector2f(x1, y2));
	shape.setPoint(2, Vector2f(x2, y2));
	shape.setPoint(3, Vector2f(x2, y1));
	w.draw(shape);
}

class Car {
protected:
    int x;
	int y;
public:
	sf::Texture tex;
	sf::Sprite spr;
	Car(int x, int y) {
		this->x = x;
		this->y = y;
	}
	virtual void move() = 0;
	void setX(int x) {
		this->x = x;
	}
	void setY(int y) {
		this->y = y;
	}
	void setxy(int x, int y) {
		setX(x);
		setY(y);
	}
	int getX() {
		return x;
	}
	int getY() {
		return y;
	}
};

class PlayerCar : public Car {
public:
	PlayerCar(int x = 115, int y = height - 100) : Car(x, y) {
		tex.loadFromFile("resources/playercar.png");
		spr.setTexture(tex, true);
		spr.setPosition(x, y);
	}
	void move() {
	}
	void moveRight() {
		if (x < 640) {
			x += 6;
			spr.setPosition(x, y);
		}
	}
	void moveLeft() {
		if (x >= 110) {
			x -= 6;
			spr.setPosition(x, y);
		}
	}
	void moveUp() {
		if (y > 6) {
			y -= 2;
			spr.setPosition(x, y);
		}
	}
	void moveDown() {
		if (y < 494) {
			y += 2;
			spr.setPosition(x, y);
		}
	}
};

class ObstacleCar : public Car {
public:
	ObstacleCar(int x = 115, int y = 0) : Car(x, y) {
	    int type = my_rand(1, 6);
		std::stringstream ss;
		ss << type;
		tex.loadFromFile("resources/" + ss.str() + ".png");
		spr.setTexture(tex, true);

		// Make sure that currently spawned car is not on same track as last one
	    do {
            x = my_rand(1,6);
	    } while (x == lastcartrack);

	    lastcartrack = x;
	    x = x * 110;
		setxy(x, y);
		spr.setPosition(x, y);
	}

	void move() {
		y = y + 1.5*velocity;
		spr.setPosition(x, y);
	}
};

int main(void)
{
	init();
	RenderWindow app(VideoMode(width, height), "Car Game");
	app.setFramerateLimit(60);
	app.setKeyRepeatEnabled(false);

	PlayerCar pCar;
	ObstacleCar Enemy1[noOfobs];

	// Score part starts here
	sf::Font font;
	if (!font.loadFromFile("resources/arial.ttf"))
	{
		return 1;
	}
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(24);
	text.setStyle(sf::Text::Bold | sf::Text::Underlined);
	text.setFillColor(sf::Color::White);
	text.setPosition(345, 10);
	// Score part ends here

	Event e;

    sf::Texture grassTex;
    if (!grassTex.loadFromFile("resources/grass.jpg")) {
        return 1;
    }
    sf::Sprite grassSpr1, grassSpr2;
    grassSpr1.setTexture(grassTex, true);
    grassSpr1.setPosition(-10, 0);
    grassSpr2.setTexture(grassTex, true);

    // Sound
    sf::SoundBuffer crashBuffer;
    if (!crashBuffer.loadFromFile("resources/carcrash.wav")) {
        return 1;
    }

    sf::Sound crashSound;
    crashSound.setBuffer(crashBuffer);

	while (app.isOpen() && !Keyboard::isKeyPressed(Keyboard::Escape))
	{
		if (!started) {
            while (app.pollEvent(e))
            {
                if (e.type == Event::Closed)
                {
                    app.close();
                }
            }
            sf::Texture startTex;
            if (!startTex.loadFromFile("resources/startpage.jpg")) {
                return 1;
            }
            sf::Sprite startSpr;
            startSpr.setTexture(startTex, true);
            startSpr.setPosition(0,0);
            app.draw(startSpr);
            if (Keyboard::isKeyPressed(Keyboard::Return)) {
                started = true;
            }
            app.display();
            c.restart();

		} else {
            while (app.pollEvent(e))
            {
                if (e.type == Event::Closed)
                {
                    app.close();
                }
            }
            app.clear();
            drawQuad(app, gray, 90, 0, 710, height);

            grassSpr2.setPosition(710, 0);
            app.draw(grassSpr1);
            app.draw(grassSpr2);

            for (int j=0; j<6; j++) {
                for (int i=0; i<5; i++) {
                    drawQuad(app, sf::Color::White, 100 + trackW + ((trackW + segW)*i), in[j], 100 + trackW + segW + ((trackW + segW)*i), in[j] + 50);
                }
                moveSeg(in[j]);
            }

            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                pCar.moveRight();
            }
            else if (Keyboard::isKeyPressed(Keyboard::Left)) {
                pCar.moveLeft();
            }
            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                pCar.moveUp();
            } else if (Keyboard::isKeyPressed(Keyboard::Down)) {
                pCar.moveDown();
            }
            int vUp;
            if (check <= 0) {
                obsP[temp] = 1;
                if (temp < noOfobs) {
                    temp++;
                }
                else {
                    temp = temp % noOfobs;
                    temp++;
                }
                // Decrease car spawn time after every 5 seconds
                check = 60 - (vUp * 2);
            }
            check--;
            int tempy, tempx;
            for (int aa = 0;aa < noOfobs;aa++) {
                if (obsP[aa] == 1) {
                    if (Enemy1[aa].getY() > 600) {
                        obsP[aa] = 0;
                        do {
                            tempx = my_rand(1,6);
                        } while (tempx == lastcartrack);
                        Enemy1[aa].setxy(tempx, 0);
                    }
                }
            }

            //CHECK COLLISION
            for (int aa = 0;aa < noOfobs;aa++) {
                if (obsP[aa] == 1) {
                    tempx = Enemy1[aa].getX();
                    tempy = Enemy1[aa].getY();
                    ptempx = pCar.getX();
                    ptempy = pCar.getY();

                    if (tempx > ptempx) {
                        if ((tempx - ptempx) < 50) {
                            if (tempy > ptempy) {
                                if ((tempy - ptempy) < 100) {
                                    if (!collision)
                                        collisionNow = true;
                                    collision = true;

                                }
                            } else {
                                if ((ptempy - tempy) < 100) {
                                    if (!collision)
                                        collisionNow = true;
                                    collision = true;
                                }
                            }
                        }
                    } else {
                        if ((ptempx - tempx) < 45) {
                            if (tempy > ptempy) {
                                if ((tempy - ptempy) < 100) {
                                    if (!collision)
                                        collisionNow = true;
                                    collision = true;
                                }
                            }
                            else {
                                if ((ptempy - tempy) < 100) {
                                    if (!collision)
                                        collisionNow = true;
                                    collision = true;
                                }
                            }
                        }
                    }
                }
            }
            if (collisionNow && collision) {
                crashSound.play();
                collisionNow = false;
            }

            for (int aa = 0;aa < noOfobs;aa++) {
                if (obsP[aa] == 1) {
                    Enemy1[aa].move();
                }
            }
            sf::Time t = c.getElapsedTime();
            std::stringstream ss2;

            if (collision) {
                velocity = 0;
                app.clear();
                sf::Texture gOverTex;
                if (!gOverTex.loadFromFile("resources/gameover.jpg")) {
                    return 1;
                }
                sf::Sprite gOverSpr;
                gOverSpr.setTexture(gOverTex, true);
                app.draw(gOverSpr);
                text.setCharacterSize(48);

                //center text
                sf::FloatRect textRect = text.getLocalBounds();
                text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
                text.setPosition(sf::Vector2f(width/2.0f, height/2.0f));

                ss2 << (int)score;
                text.setString("Score: " + ss2.str());
            } else {
                ss2 << (int)t.asSeconds();
                text.setString("Score: " + ss2.str());
                app.draw(pCar.spr);
                score = t.asSeconds();
            }

            // Increase velocity
            if (velocity != 0) {
                for (int aa = 0; aa < noOfobs; aa++) {
                    if (obsP[aa] == 1) {
                        app.draw(Enemy1[aa].spr);
                    }
                }

                velocity = 3;
                vUp = (int)t.asSeconds() / 8;
                velocity = 3 + vUp;
            }

            app.draw(text);
            app.display();
		}
	}
	return 0;
}
