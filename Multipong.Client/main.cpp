
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include <vector>
#include <iostream>
#include <filesystem>
#include <functional>

enum class ViewType
{
	MainMenu,
	NewGame,
	JoinGame,
	PlayGame,
	PauseHost,
	PauseClient,
	Unknown
};

class Simulatable // interface
{
public:
	Simulatable() = default;
	virtual ~Simulatable() = default;
	virtual void simulate(float& fixed_delta) = 0;
};

class Drawable // interface
{
public:
	Drawable() = default;
	virtual ~Drawable() = default;
	virtual void draw(sf::RenderWindow& window, float& alfa_time) = 0;
};

class Interactive // interface
{
public:
	Interactive() = default;
	virtual ~Interactive() = default;
	virtual void onClick() = 0;
	virtual void hoveredOver() = 0;
	virtual void notHoveredOver() = 0;
	virtual bool checkMouseOverlap(sf::Vector2f& mouse_wrld_pos) = 0;
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

class Paddle : public Simulatable, public Drawable
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

class Ball : public Simulatable, public Drawable
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
		if (pos_.x <= 0 - 2 * shape_.getRadius())
		{
			return 2;
		}
		else if (pos_.x >= 1920)
		{
			return 1;
		}
		else
		{
			return 0;
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

class Button : public Drawable, public Interactive
{
private:
	sf::RectangleShape shape_;
	sf::Font font_;
	sf::Text text_;

	std::function<void()> on_click_;
public:
	Button(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Color color, std::function<void()> on_click) : font_(sf::Font("Fonts/Scifi2k2.ttf")), text_(sf::Text(font_, text, size.y - size.y / 2)), on_click_(on_click)
	{
		shape_.setPosition(position);
		shape_.setSize(size);
		shape_.setFillColor(color);

		text_.setOrigin(text_.getLocalBounds().getCenter());
		text_.setPosition(shape_.getGlobalBounds().getCenter());

		dimDown();
	}

	virtual ~Button() = default;

	void draw(sf::RenderWindow& window, float& alfa_time)
	{
		window.draw(shape_);
		window.draw(text_);
	}

	void hoveredOver()
	{
		lightUp();
	}

	void notHoveredOver()
	{
		dimDown();
	}

	virtual void onClick()
	{
		on_click_();
	}

	void lightUp()
	{
		sf::Color color = shape_.getFillColor();
		color.a = 200;
		shape_.setFillColor(color);

		sf::Color text_color = text_.getFillColor();
		text_color.a = 255;
		text_.setFillColor(text_color);
	}

	void dimDown()
	{
		sf::Color color = shape_.getFillColor();
		color.a = 155;
		shape_.setFillColor(color);

		sf::Color text_color = text_.getFillColor();
		text_color.a = 155;
		text_.setFillColor(text_color);
	}

	virtual bool checkMouseOverlap(sf::Vector2f& mouse_wrld_pos)
	{
		return(shape_.getGlobalBounds().contains(mouse_wrld_pos));
	}
};

class Textbox : public Drawable
{
protected:
	sf::RectangleShape shape_;
	sf::Font font_;
	sf::Text text_;
	std::string& string_;

public:
	Textbox(sf::Vector2f position, sf::Vector2f size, std::string& str) : font_(sf::Font("Fonts/Scifi2k2.ttf")), text_(sf::Text(font_, str, size.y - size.y / 2)), string_(str)
	{
		shape_.setPosition(position);
		shape_.setSize(size);

		centerText();
	}

	virtual ~Textbox() = default;

	virtual void draw(sf::RenderWindow& window, float& alfa_time)
	{
		window.draw(text_);
	}

	void update()
	{
		text_.setString(sf::String(string_));
	}

	void centerText()
	{
		text_.setOrigin(text_.getLocalBounds().getCenter());
		text_.setPosition(shape_.getGlobalBounds().getCenter());
	}
};

class View
{
protected:
	std::vector<Simulatable*> simulatables_;
	std::vector<Drawable*> drawables_;
	std::vector<Interactive*> interactives_;

	ViewType type = ViewType::Unknown;

public:
	View() = default;
	virtual ~View() = default;

	ViewType getType() { return type; }

	void draw(sf::RenderWindow& window, float& alfa_time)
	{
		for (Drawable* d : drawables_)
		{
			d->draw(window, alfa_time);
		}
	}

	void process(sf::Vector2f& mouse_wrld_pos, bool& was_mouse_unclicked)
	{
		for (Interactive* i : interactives_)
		{
			if (i->checkMouseOverlap(mouse_wrld_pos))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && was_mouse_unclicked)
				{
					i->onClick();
					was_mouse_unclicked = false;
				}

				i->hoveredOver();
			}
			else
			{
				i->notHoveredOver();
			}
		}
	}
};

class ViewManager
{
private:
	std::vector<View*> views_;
	View* active_ = nullptr;
	bool was_mouse_unclicked_ = true;
	bool is_host_ = false;

public:
	ViewManager() = default;
	~ViewManager() = default;

	void setActive(ViewType type)
	{
		for (int i = 0; i < views_.size(); i++)
		{
			if (views_[i]->getType() == type)
			{
				active_ = views_[i];

				if (views_[i]->getType() == ViewType::NewGame)
				{
					is_host_ = true;
				}
				else if (views_[i]->getType() == ViewType::JoinGame)
				{
					is_host_ = false;
				}

				return;
			}
		}
	}

	void drawActiveView(sf::RenderWindow& window, float& alfa_time)
	{
		if (active_)
		{
			active_->draw(window, alfa_time);
		}
	}

	void addView(View* view)
	{
		views_.push_back(view);
	}

	void processActiveView(sf::Vector2f& mouse_wrld_pos)
	{
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			was_mouse_unclicked_ = true;
		}

		active_->process(mouse_wrld_pos, was_mouse_unclicked_);
	}

	bool& getIsHost()
	{
		return is_host_;
	}
};

class MainMenu : public View
{
public:
	MainMenu(ViewManager& view_manager, sf::RenderWindow& window)
	{
		type = ViewType::MainMenu;
		std::string game_logo_str = "Multipong";

		Textbox* game_logo = new Textbox(sf::Vector2f{ 0, 25 }, sf::Vector2f{ 1920, 500}, game_logo_str);

		Button* start_new = new Button(sf::Vector2f{ 710, 390 }, sf::Vector2f{ 500, 50 }, "Start New", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::NewGame); });
		Button* join = new Button(sf::Vector2f{ 710, 490 }, sf::Vector2f{ 500, 50 }, "Join Game", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::JoinGame); });
		Button* exit = new Button(sf::Vector2f{ 710, 590 }, sf::Vector2f{ 500, 50 }, "Exit", sf::Color::Blue, [&window]() { window.close(); });

		drawables_.push_back(game_logo);
		drawables_.push_back(start_new);
		drawables_.push_back(join);
		drawables_.push_back(exit);

		interactives_.push_back(start_new);
		interactives_.push_back(join);
		interactives_.push_back(exit);
	}

	~MainMenu() = default;
};

class NewGame : public View
{
public:
	NewGame(ViewManager& view_manager)
	{
		type = ViewType::NewGame;

		Button* host = new Button(sf::Vector2f{ 710, 490 }, sf::Vector2f{ 500, 50 }, "Host", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::PlayGame); });
		Button* go_back = new Button(sf::Vector2f{ 710, 590 }, sf::Vector2f{ 500, 50 }, "Return", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::MainMenu); });

		drawables_.push_back(host);
		drawables_.push_back(go_back);

		interactives_.push_back(host);
		interactives_.push_back(go_back);
	}

	~NewGame() = default;
};

class JoinGame : public View
{
public:
	JoinGame(ViewManager& view_manager)
	{
		type = ViewType::JoinGame;

		Button* join = new Button(sf::Vector2f{ 710, 490 }, sf::Vector2f{ 500, 50 }, "Join", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::PlayGame); });
		Button* go_back = new Button(sf::Vector2f{ 710, 590 }, sf::Vector2f{ 500, 50 }, "Return", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::MainMenu); });

		drawables_.push_back(join);
		drawables_.push_back(go_back);

		interactives_.push_back(join);
		interactives_.push_back(go_back);
	}

	~JoinGame() = default;
};

class PlayGame : public View
{
public:
	PlayGame(ViewManager& view_manager)
	{
		type = ViewType::PlayGame;
	}

	~PlayGame() = default;
};

class PauseHost : public View
{
public:
	PauseHost(ViewManager& view_manager)
	{
		type = ViewType::PauseHost;

		Button* resume = new Button(sf::Vector2f{ 710, 490 }, sf::Vector2f{ 500, 50 }, "Resume", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::PlayGame); });
		Button* close = new Button(sf::Vector2f{ 710, 590 }, sf::Vector2f{ 500, 50 }, "Close Server", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::MainMenu); });

		drawables_.push_back(resume);
		drawables_.push_back(close);

		interactives_.push_back(resume);
		interactives_.push_back(close);
	}

	~PauseHost() = default;
};

class PauseClient : public View
{
public:
	PauseClient(ViewManager& view_manager)
	{
		type = ViewType::PauseClient;

		Button* resume = new Button(sf::Vector2f{ 710, 490 }, sf::Vector2f{ 500, 50 }, "Resume", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::PlayGame); });
		Button* close = new Button(sf::Vector2f{ 710, 590 }, sf::Vector2f{ 500, 50 }, "Leave Server", sf::Color::Blue, [&view_manager]() { view_manager.setActive(ViewType::MainMenu); });

		drawables_.push_back(resume);
		drawables_.push_back(close);

		interactives_.push_back(resume);
		interactives_.push_back(close);
	}

	~PauseClient() = default;
};

class ScoreCounter : public Drawable
{
private:
	int score_p1_ = 0;
	int score_p2_ = 0;

	Textbox textbox_s1_;
	Textbox textbox_s2_;

	std::string string_s1_ = "N/A";
	std::string string_s2_ = "N/A";

public:
	ScoreCounter(sf::Vector2f position, sf::Vector2f size) : textbox_s1_(position, sf::Vector2f{ size.x / 2, size.y / 2 }, string_s1_), textbox_s2_(sf::Vector2f{ position.x + size.x / 2, position.y }, sf::Vector2f{ size.x / 2, size.y / 2 }, string_s2_)
	{
		updateStrings();
	}

	~ScoreCounter() = default;

	void resetScore()
	{
		score_p1_ = 0;
		score_p2_ = 0;

		updateStrings();
	}

	void givePoint(int player_index)
	{
		if (player_index != 0)
		{
			if (player_index == 1)
			{
				score_p1_++;
			}
			else if (player_index == 2)
			{
				score_p2_++;
			}

			textbox_s1_.centerText();
			textbox_s2_.centerText();

			updateStrings();
		}
	}

	int getScore(int player_index)
	{
		if (player_index == 1)
		{
			return score_p1_;
		}
		else if (player_index == 2)
		{
			return score_p2_;
		}
		else
		{
			return 0;
		}
	}

	virtual void draw(sf::RenderWindow& window, float& delta_time)
	{
		textbox_s1_.draw(window, delta_time);
		textbox_s2_.draw(window, delta_time);
	}

	void updateStrings()
	{
		string_s1_ = std::to_string(score_p1_);
		string_s2_ = std::to_string(score_p2_);

		textbox_s1_.update();
		textbox_s2_.update();
	}
};

class GameManager
{
private:
	Paddle* player_1_ = nullptr;
	Paddle* player_2_ = nullptr;

	Ball* ball = nullptr;

	bool last_collision_check_ = false;

	ScoreCounter score_counter_;

public:
	GameManager() : score_counter_(sf::Vector2f{ 510, 100 }, sf::Vector2f{ 900, 300 })
	{
		player_1_ = new Paddle(0.0f, sf::Color::Cyan);
		player_2_ = new Paddle(1870.0f, sf::Color::Magenta);
		ball = new Ball;
	}

	~GameManager()
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
			score_counter_.givePoint(1);
			ball->resetVelocity();
			ball->centerPosition();
		}
		if (ball->checkScore() == 2)
		{
			score_counter_.givePoint(2);
			ball->resetVelocity();
			ball->centerPosition();
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
		score_counter_.draw(window, alfa_time);
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

class FpsMeter : public Drawable
{
private:
	static const int number_of_samples_ = 10;
	int max_visual_update_fps_;
	float samples_[number_of_samples_];
	std::string fps_string_ = "fps: N/A";
	int next_sample_ = 0;
	Textbox textbox_;
	float time_till_next_update_ = 0;

public:
	FpsMeter(sf::Vector2f position, sf::Vector2f size, int max_fps) : textbox_(position, size, fps_string_), max_visual_update_fps_(max_fps)
	{
		time_till_next_update_ = 1.0f / max_visual_update_fps_;

		for (int i = 0; i < number_of_samples_; i++)
		{
			samples_[i] = 0.0f;
		}
	}

	~FpsMeter() = default;

	void updateString()
	{
		float sum = 0.0f;

		for (int i = 0; i < number_of_samples_; i++)
		{
			sum += samples_[i];
		}
		
		float calculated_fps = sum / number_of_samples_;

		fps_string_ = "fps: " + std::to_string(int(calculated_fps));
	}

	void takeSample(float time)
	{
		samples_[next_sample_] = 1.0f / time;

		if (next_sample_ == number_of_samples_ - 1)
		{
			next_sample_ = 0;
		}
		else
		{
			next_sample_++;
		}
	}

	virtual void draw(sf::RenderWindow& window, float& delta_time)
	{
		time_till_next_update_ -= delta_time;

		if (time_till_next_update_ < 0)
		{
			updateString();
			textbox_.update();
			time_till_next_update_ = 1.0f / max_visual_update_fps_;
		}

		textbox_.draw(window, delta_time);
	}
};

int main()
{
	sf::VideoMode mode(sf::Vector2u{ 1920, 1080 }, 32U);
	sf::RenderWindow window(mode, "Multipong", sf::Style::Close, sf::State::Fullscreen);
	window.setVerticalSyncEnabled(true);
	sf::View view(window.getDefaultView().getCenter(), sf::Vector2f{1920, 1080});
	window.setView(view);

	sf::Clock clk;

	float simulation_fps = 256.0f;
	float fixed_delta = 1.0f / simulation_fps;

	float accumulator = 0;

	GameManager game_manager;

	InputPacket packet_p1;
	InputPacket packet_p2;
	packet_p1.player_index = 1;
	packet_p2.player_index = 2;

	ViewManager view_manager;
	view_manager.addView(new MainMenu(view_manager, window));
	view_manager.addView(new NewGame(view_manager));
	view_manager.addView(new JoinGame(view_manager));
	view_manager.addView(new PlayGame(view_manager));
	view_manager.addView(new PauseHost(view_manager));
	view_manager.addView(new PauseClient(view_manager));
	view_manager.setActive(ViewType::MainMenu);

	FpsMeter fps_meter(sf::Vector2f{ 1720, 0 }, sf::Vector2f{ 200, 50 }, 3);

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

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
		{
			if (view_manager.getIsHost())
			{
				view_manager.setActive(ViewType::PauseHost);
			}
			else
			{
				view_manager.setActive(ViewType::PauseClient);
			}
		}

		window.clear(sf::Color::Black);

		accumulator += delta_time;

		while (accumulator > fixed_delta)
		{
			game_manager.takeInput(packet_p1);
			game_manager.takeInput(packet_p2);

			game_manager.simulate(fixed_delta);

			sf::Vector2i mouse_pos = sf::Mouse::getPosition();
			sf::Vector2f mouse_wrld_pos = window.mapPixelToCoords(mouse_pos);

			view_manager.processActiveView(mouse_wrld_pos);
			accumulator -= fixed_delta;
		}

		float alfa_time = accumulator / fixed_delta;
		game_manager.draw(window, alfa_time);
		fps_meter.draw(window, delta_time);
		view_manager.drawActiveView(window, alfa_time);

		fps_meter.takeSample(delta_time);
		window.display();
	}

	return 0;
}