cc -fsanitize=thread -g

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./philo 1 800 200 200
valgrind --tool=helgrind ./philo 5 400 200 600
valgrind --tool=drd ./philo 5 400 200 600

## Philosophers visualizer
https://nafuka11.github.io/philosophers-visualizer/ 

timeout 10s