
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include <vector>
#include <iostream>

class Simulatable // interface
{
public:
	Simulatable() = default;
	~Simulatable() = default;
	virtual void simulate(float& fixed_delta) = 0;
	virtual void draw(sf::RenderWindow& window, float& alfa_time) = 0;
};

enum class PacketType
{
	InputPacket
};

struct InputPacket
{
	PacketType type = PacketType::InputPacket;
	int player_index = 0;
	bool w = false;
	bool s = false;
};

class Ball;

class Paddle : public Simulatable
{
private:
	sf::RectangleShape shape_;
	float vmax_ = 500; // px/s
	float velocity_y_ = 0; // px/s
	sf::Vector2f pos_;
	sf::Vector2f previous_pos_;

	friend bool checkCollision(Paddle& player, Ball& ball);

public:
	Paddle(float pos_x, sf::Color color) : pos_({ pos_x, 390.0f })
	{
		shape_ = sf::RectangleShape(sf::Vector2f{ 50.0f, 300.0f });
		shape_.setPosition(pos_);
		shape_.setFillColor(color);
	}

	~Paddle() = default;

	virtual void simulate(float& fixed_delta)
	{
		previous_pos_ = pos_;
		pos_.y += velocity_y_ * fixed_delta;

		if (pos_.y < 0)
		{
			pos_.y = 0;
		}
		else if (pos_.y > 1080 - shape_.getSize().y)
		{
			pos_.y = 1080 - shape_.getSize().y;
		}

		moveStop();
	}

	void moveUp()
	{
		velocity_y_ = -vmax_;
	}

	void moveDown()
	{
		velocity_y_ = vmax_;
	}

	void moveStop()
	{
		velocity_y_ = 0;
	}

	virtual void draw(sf::RenderWindow& window, float& alfa_time)
	{
		shape_.setPosition(pos_ + ((pos_ - previous_pos_) * alfa_time));
		window.draw(shape_);
	}
};

class Ball : public Simulatable
{
private:
	sf::CircleShape shape_;
	sf::Vector2f velocity_;
	sf::Vector2f pos_;
	sf::Vector2f previous_pos_;

	friend bool checkCollision(Paddle& player, Ball& ball);

public:
	Ball()
	{
		shape_.setRadius(20.0f);
		shape_.setFillColor(sf::Color::White);
		centerPosition();
		resetVelocity();
	}

	~Ball() = default;

	virtual void simulate(float& fixed_delta)
	{
		previous_pos_ = pos_;
		pos_ += velocity_ * fixed_delta;

		if (pos_.y <= 0)
		{
			pos_.y = 0;
			changeDirectionY();
		}
		if (pos_.y >= 1080 - (shape_.getRadius() * 2))
		{
			pos_.y = 1080 - (shape_.getRadius() * 2);
			changeDirectionY();
		}
	}

	virtual void draw(sf::RenderWindow& window, float& alfa_time)
	{
		shape_.setPosition(pos_ + ((pos_ - previous_pos_) * alfa_time));
		window.draw(shape_);
	}

	int checkScore()
	{
		if (pos_.x <= 0 - shape_.getRadius())
		{
			centerPosition();
			return 2;
		}
		if (pos_.x >= 1920 + shape_.getRadius())
		{
			centerPosition();
			return 1;
		}
	}

	void centerPosition()
	{
		pos_ = sf::Vector2f{ (1920.0f - shape_.getRadius() * 2) / 2, (1080.0f - shape_.getRadius() * 2) / 2 };
		previous_pos_ = pos_;
	}

	void changeDirectionX()
	{
		velocity_.x = -1 * velocity_.x;
	}

	void changeDirectionY()
	{
		velocity_.y = -1 * velocity_.y;
	}

	void speedUp()
	{
		velocity_.x = 1.1f * velocity_.x;
	}

	void resetVelocity()
	{
		velocity_ = sf::Vector2f{ 400.0f, 200.0f };
	}
};

bool checkCollision(Paddle& player, Ball& ball)
{
	if (player.shape_.getGlobalBounds().findIntersection(ball.shape_.getGlobalBounds()))
		return true;
	else
		return false;
}

class Game
{
private:

	Paddle* player_1_ = nullptr;
	Paddle* player_2_ = nullptr;

	int score_p1_ = 0;
	int score_p2_ = 0;

	Ball* ball = nullptr;

	bool last_collision_check_ = false;

public:
	Game()
	{
		player_1_ = new Paddle(0.0f, sf::Color::Cyan);
		player_2_ = new Paddle(1870.0f, sf::Color::Magenta);
		ball = new Ball;
	}

	~Game()
	{
		delete player_1_;
		player_1_ = nullptr;

		delete player_2_;
		player_2_ = nullptr;

		delete ball;
		ball = nullptr;
	}

	void simulate(float delta_time)
	{
		player_1_->simulate(delta_time);
		player_2_->simulate(delta_time);
		ball->simulate(delta_time);

		if (ball->checkScore() == 1)
		{
			score_p1_++;
			ball->resetVelocity();
		}
		if (ball->checkScore() == 2)
		{
			score_p2_++;
			ball->resetVelocity();
		}
		
		if (checkCollision(*player_1_, *ball) || checkCollision(*player_2_, *ball))
		{
			if (!last_collision_check_)
			{
				ball->changeDirectionX();
				ball->speedUp();
				last_collision_check_ = true;
			}
		}
		else
		{
			last_collision_check_ = false;
		}
	}

	void draw(sf::RenderWindow& window, float& alfa_time)
	{
		player_1_->draw(window, alfa_time);
		player_2_->draw(window, alfa_time);
		ball->draw(window, alfa_time);
	}

	void takeInput(InputPacket packet)
	{
		if (packet.player_index == 1)
		{
			if (packet.w)
			{
				player_1_->moveUp();
				return;
			}
			else if (packet.s)
			{
				player_1_->moveDown();
				return;
			}
		}
		else if (packet.player_index == 2)
		{
			if (packet.w)
			{
				player_2_->moveUp();
				return;
			}
			else if (packet.s)
			{
				player_2_->moveDown();
				return;
			}
		}
	}
};

int main()
{
	sf::VideoMode mode(sf::Vector2u{ 1920, 1080 }, 32U);
	sf::RenderWindow window(mode, "Multipong", sf::Style::Close, sf::State::Windowed);
	window.setVerticalSyncEnabled(true);

	sf::Clock clk;

	float simulation_fps = 60.0f;
	float fixed_delta = 1.0f / simulation_fps;

	float accumulator = 0;

	Game game;

	InputPacket packet_p1;
	InputPacket packet_p2;
	packet_p1.player_index = 1;
	packet_p2.player_index = 2;

	while (window.isOpen())
	{
		float delta_time = clk.restart().asSeconds();

		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			packet_p1.w = true;
		}
		else
		{
			packet_p1.w = false;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			packet_p1.s = true;
		}
		else
		{
			packet_p1.s = false;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			packet_p2.w = true;
		}
		else
		{
			packet_p2.w = false;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			packet_p2.s = true;
		}
		else
		{
			packet_p2.s = false;
		}

		window.clear(sf::Color::Black);

		accumulator += delta_time;

		while (accumulator > fixed_delta)
		{
			game.takeInput(packet_p1);
			game.takeInput(packet_p2);

			game.simulate(fixed_delta);
			accumulator -= fixed_delta;
		}

		float alfa_time = accumulator / fixed_delta;
		game.draw(window, alfa_time);

		window.display();
	}

	return 0;
}