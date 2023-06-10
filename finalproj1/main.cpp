
#include "Game.h"


using namespace sf;
using namespace std;




int main()
{
	// Create the main window
	RenderWindow window(VideoMode(640, 384), "Legend Quest 2D");
	
	Game game;
	game.Init(window);

	Clock clock;

	// Start the game loop 
	while (window.isOpen())
	{
		
		bool fire = false;
		char key = -1;
		// Process events
		Event event;
		while (window.pollEvent(event))
		{
			// Close window: exit
			if (event.type == Event::Closed) 
				window.close();
			else if (event.type == Event::TextEntered)
			{
				if (event.text.unicode == GC::ESCAPE_KEY)
					window.close(); 
				if (isdigit(event.text.unicode) || isalpha(event.text.unicode))
					key = static_cast<char>(event.text.unicode);
				else if (event.text.unicode == GC::BACKSPACE_KEY && !game.metrics.name.empty())
					game.metrics.name = game.metrics.name.substr(0, game.metrics.name.length() - 1);
			}
			else if (event.type == Event::KeyReleased)
			{
				if (event.key.code == Keyboard::Space)
					fire = true; 
				//this isn't the only way to monitor for a fire key press, 
				//could also use Keyboard::isKeyPressed(Keyboard::Space), either is fine
				//isKeyPressed() can be called from anywhere, but doesn't wait for you to let go
			}
		} 

		// Clear screen
		window.clear();

		float elapsed = clock.getElapsedTime().asSeconds();
		clock.restart();
		
		game.Update(window, elapsed, fire, key);
		game.Render(window, elapsed);
		
		// Update the window
		window.display();
	}

	return EXIT_SUCCESS;
}
