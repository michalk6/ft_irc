NAME      = ircserv
CXX       = c++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98 -I./inc -pedantic
MAKEFLAGS += --no-print-directory -s
SRCS_DIR  = src
SRCS      = $(addprefix $(SRCS_DIR)/, main.cpp Server.cpp)
OBJS      = $(SRCS:.cpp=.o)
RM        = rm -f

all: $(NAME)
	@echo "Program ready to use."

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	@echo "Object files removed."

fclean: clean
	$(RM) $(NAME)
	@echo "Fully cleaned up."

re: fclean all

debug: CXXFLAGS += -g -fsanitize=address
debug: re
	@echo "Program ready to use (debug mode)"

.PHONY: all clean fclean re debug
