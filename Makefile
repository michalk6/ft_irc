NAME      = ircserv
CXX       = c++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98 -I./inc -pedantic
MAKEFLAGS += --no-print-directory -s
SRCS_DIR  = src
OBJS_DIR  = obj
SRCS      = $(addprefix $(SRCS_DIR)/,	\
			main.cpp					\
			Server.cpp					\
			Client.cpp					\
			)
OBJS      = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)
RM        = rm -f

all: $(NAME)
	@echo "\033[38;5;195m--------------------------\033[0m"
	@echo "\033[38;5;195m|         IRCSERV        |\033[0m"
	@echo "\033[38;5;195m--------------------------\033[0m"
	@echo "\033[38;5;154mProgram ready to use.\033[0m"

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $@

clean:
	$(RM) -r $(OBJS_DIR)
	@echo "\033[38;5;166mObject files removed.\033[0m"

fclean: clean
	$(RM) $(NAME)
	@echo "\033[38;5;166mFully cleaned up.\033[0m"

re: fclean all

debug: CXXFLAGS += -g -fsanitize=address
debug: re
	@echo "\033[38;5;220m--- (debug mode) --- \033[0m"

.PHONY: all clean fclean re debug
