
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include <vector>;
#include <iostream>;


class Simulatable // interface
{
public:
	Simulatable() = default;
	~Simulatable() = default;
	virtual void simulate(float& delta_time) = 0;
	virtual void draw(sf::RenderWindow& window) = 0;
};

struct Input
{
	bool w = false;
	bool s = false;
};

class Paddle : public Simulatable
{
private:
	sf::RectangleShape shape_;
	Input input_;
	float speed_ = 100; // px/s;
	sf::Vector2f pos_;

public:
	Paddle(float pos_x) : pos_({ pos_x, 0.0f })
	{
		shape_ = sf::RectangleShape(sf::Vector2f{ 50.0f, 300.0f });
		shape_.setPosition(pos_);
		shape_.setFillColor(sf::Color::Cyan);
	}

	~Paddle() = default;

	void simulate(float& delta_time)
	{
		if (input_.w && !input_.s)
		{
			pos_.y -= speed_ * delta_time;
		}
		else if (input_.s && !input_.w)
		{
			pos_.y += speed_ * delta_time;
		}
		else
		{

		}

		shape_.setPosition(pos_);
	}

	void moveUp()
	{
		input_.w = true;
		input_.s = false;
		std::cout << "moving up" << std::endl;
	}

	void moveDown()
	{
		input_.w = false;
		input_.s = true;
		std::cout << "moving down" << std::endl;
	}

	void moveStop()
	{
		input_.w = false;
		input_.s = false;
		std::cout << "stopped" << std::endl;
	}

	void draw(sf::RenderWindow& window)
	{
		window.draw(shape_);
	}
};

struct Context
{
	std::vector<Simulatable*> sims;
	sf::RenderWindow window;
	float delta_time = 0.001f;

	Paddle* player_1 = nullptr;
	Paddle* player_2 = nullptr;

	Context()
	{
		sf::VideoMode mode(sf::Vector2u{ 1920, 1080 }, 32U);
		window = sf::RenderWindow(mode, "Multipong", sf::Style::Default, sf::State::Windowed);
	}

	~Context() = default;
};

enum class Status
{
	Success,
	Failure
};

class Game
{
private:
	Context context_;
	int controlled = 1;

public:
	Game() = default;
	~Game() = default;

	sf::RenderWindow& getWindow() { return context_.window; }

	void simulateAll()
	{
		for (int i = 0; i < context_.sims.size(); i++)
		{
			context_.sims[i]->simulate(context_.delta_time);
		}
	}

	void drawAll()
	{
		for (int i = 0; i < context_.sims.size(); i++)
		{
			context_.sims[i]->draw(context_.window);
		}
	}

	Status addPlayer()
	{
		if (!context_.player_1)
		{
			context_.player_1 = new Paddle(0.0f);
			context_.sims.push_back(context_.player_1);
			return Status::Success;
		}
		else if (!context_.player_2)
		{
			context_.player_2 = new Paddle(context_.window.getSize().x - 50);
			context_.sims.push_back(context_.player_2);
			return Status::Success;
		}
		else
		{
			return Status::Failure;
		}
	}

	void inputPlayer()
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			if (controlled == 1)
			{
				context_.player_1->moveUp();
			}
			if (controlled == 2)
			{
				context_.player_2->moveUp();
			}
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			if (controlled == 1)
			{
				context_.player_1->moveDown();
			}
			if (controlled == 2)
			{
				context_.player_2->moveDown();
			}
		}
		else
		{
			if (controlled == 1)
			{
				context_.player_1->moveStop();
			}
			if (controlled == 2)
			{
				context_.player_2->moveStop();
			}
		}
	}
};

int main()
{
	Game game;
	game.addPlayer();
	//game.addPlayer();

	while (game.getWindow().isOpen())
	{
		while (const std::optional event = game.getWindow().pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				game.getWindow().close();
			}
		}

		game.inputPlayer();

		game.getWindow().clear();

		game.simulateAll();
		game.drawAll();

		game.getWindow().display();
	}

	return 0;
}