/* Macchina di Turing universale - UTM
 *
 * Gli stati sono salvati ognuno in un array di puntatori, ogni puntatore punta
 * ad un array in cui l'elemento 0 è il carattere da leggere, e ogni tre celle
 * si ripete lo stesso schema: carattere da scrivere, movimento testina, prossimo
 * stato.
 * int ***states;
 * int **states[i];
 * int *states[i][j];
 * int states[i][j][0]
 * Per segnalare la fine delle transizioni, ogni array ha le tre celle finali
 * inizializzate a 0. Ogni stato ha una transizione in più, vuota, inizializzata
 * a zero, per segnalare che non possono essere letti altri caratteri.
 *
 * Gli stati di accettazione sono creati vuoti, e, inoltre il loro numero è
 * salvato in un array.
 *
 * Ogni riga dell'input è passata alla funzione step().
 * Essa implementa una pila dinamica, ogni cella della pila è una struct che
 * memorizza stato, input e sua lunghezza, e posizione sull'input.
 * Quando la testa della coda raggiunge l'elemento 'level' la funzione ha completato
 * la visita in ampiezza di un livello dell'albero delle computazioni, quindi
 * ha compiut un passo.
 * Se la testa raggiunge l'ultima cella occupata non è stata trovata una
 * computazione di accettazione e quindi si rifiuta
 * Il nastro di input è memorizzato in un array che tiene conto di ogni nastro e di
 * quanti nodi hanno un riferimento ad esso. In questo modo per ogni transizione 
 * deterministica non c'è bisogno di duplicare il nastro ma semplicemente 
 * riferirsi ad esso. Ogni nastro ha un suo contatore che quando scende a zero 
 * indica che nessun nodo utilizza quel nastro e può essere liberato.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <useful.h>	// Defines a function reader() to read an arbitrarily long string from a stream.
					// It return 0 if the read was successful

typedef struct taper taper;

struct taper {
	int count;
	char *tape;
};

typedef struct config config;

struct config {
	int state;
	int head;
	int input_len;
	taper *mine;
	struct config *next;
};

int ***states;
char *input, output;
int *acceptance_state, acc_max, testina, input_len, max_runs, max_states;

/* Accepts the current header position and enlarges the input string with blanks */
int widen(char **new_input, int *testina, int *input_len) {
	char *blanks = "__________";
	int len = strlen(blanks);
	char *buffer = malloc((2 * len + *input_len + 1) * sizeof(char));
	if (!buffer)
		return EXIT_FAILURE;
	buffer[0] = '\0';
	strncat(buffer, blanks, len);
	strncat(buffer, *new_input, *input_len);
	strncat(buffer, blanks, len);
	free(*new_input);
	*new_input = buffer;
	*testina += len;
	*input_len = strlen(*new_input);
	return EXIT_SUCCESS;
}

void print_states(void) {
	int i, j;
	for (i = 0; i < max_states; i++) {
		printf("State:\t%d\tRead: %c\n", i, states[i][0][0]);
		if (states[i][0][0] == 0)
			continue;
		for (j = 1; states[i][j][0] != 0; j++) {
			printf("\t\tRead: %c\n", states[i][j][0]);
		}
	}
}

int step(char *input, int header, int length, int state, int run) {

	int i, j;

	int max_stringhe = 6, upper = 1;
	taper **stringhe = malloc(max_stringhe * sizeof(taper *));

	int testina, steps, lunghezza, stato;
	testina = header;
	steps = run;
	lunghezza = length;
	stato = state;

	stringhe[0] = malloc(sizeof(taper));
	stringhe[0]->tape = malloc((lunghezza + 1) * sizeof(char));
	strcpy(stringhe[0]->tape, input);
	stringhe[0]->count = 1;

	config *examining, *level, *last, *tmp;
	examining = level = last = NULL;

	examining = calloc(1, sizeof(config));
	examining->next = malloc(sizeof(config));
	last = examining->next;
	level = examining;
	examining->mine = stringhe[0];

	while (output != '1' && steps) {

		/* Check if the queue is empty */
		if (stato == -2) {	// New nodes are initialized with number -2, this way if we get to a -2 we know there are no more nodes
			if (output != 'U')
				output = '0';
			break;
		}

		/* Check if we're in an acceptance state */
		for (i = 0; i < acc_max; i++) {
			if (stato == acceptance_state[i]) {
				output = '1';
				break;
			}
		}
		if (output == '1')
			break;

		/* Check if we are in a loop */
		if (stato == -1) {
			output = 'U';

			/* Check if we are in the last state of the level */
			if (examining == level) {
				steps--;
				level = last;
			}

			/* Check if the queue is empty */
			if (examining == last)
				break;

			/* Move to the next state */
			tmp = examining;
			examining = examining->next;
			if (tmp->mine->count-- == 0) {
				free(tmp->mine->tape);
				free(tmp->mine);
			}
			free(tmp);
			stato = examining->state;
			lunghezza = examining->input_len;
			testina = examining->head;

			continue;
		}
		if (examining == last)
			break;

		/* Check if the state exists and has available transitions */
		if (stato >= max_states || states[stato][0][0] == 0) {
			if (output != 'U')
				output = '0';

			/* Check if we're in the last node of the level */
			if (examining == level) {
				steps--;
				level = last;
			}

			/* Check if the queue is empty */
			if (examining == last)
				break;

			/* Move to the next state */
			tmp = examining;
			examining = examining->next;
			if (tmp->mine->count-- == 0) {
				free(tmp->mine->tape);
				free(tmp->mine);
			}
			free(tmp);
			stato = examining->state;
			lunghezza = examining->input_len;
			testina = examining->head;

			continue;
		}
		if (examining == last)
			break;

		/* Search for the read in the state */
		for (i = 0; states[stato][i][0] != examining->mine->tape[testina] && states[stato][i][0] != 0; i++)
			;
		if (states[stato][i][0] == 0) {
			if (output != 'U')
				output = '0';

			/* Check if we're in the last node of the level */
			if (examining == level) {
				steps--;
				level = last;
			}

			/* Check if the queue is empty */
			if (examining == last)
				break;

			/* Move to the next state */
			tmp = examining;
			examining = examining->next;
			if (tmp->mine->count-- == 0) {
				free(tmp->mine->tape);
				free(tmp->mine);
			}
			free(tmp);
			stato = examining->state;
			lunghezza = examining->input_len;
			testina = examining->head;

			continue;
		}
		if (examining == last)
			break;

		/* For every transition of the state enqueue a new node/struct */
		for (j = 1; states[stato][i][j]; j += 3) {

			/* Allocate, copy and modify the tape */
			if (states[stato][i][4] == 0) {
				last->mine = examining->mine;
				last->mine->count++;
			}
			else {
				stringhe[upper] = malloc(sizeof(taper));
				stringhe[upper]->tape = malloc((lunghezza + 1) * sizeof(char));
				strcpy(stringhe[upper]->tape, examining->mine->tape);
				stringhe[upper]->count = 0;
				last->mine = stringhe[upper];
				upper++;
				if (upper >= max_stringhe) {
					stringhe = realloc(stringhe, (max_stringhe + 5) * sizeof(taper *));
					max_stringhe += 5;
				}
			}
			last->mine->tape[testina] = states[stato][i][j];
			last->input_len = lunghezza;

			/* Move the head */
			last->head = (testina + states[stato][i][j + 1]);
			if (last->head < 0 || last->head >= last->input_len)
				widen(&(last->mine->tape), &(last->head), &(last->input_len));

			/* Give the new state its number */
			last->state = states[stato][i][j + 2];

			/* Allocate a new node */
			last->next = malloc(sizeof(config));
			last = last->next;
			last->state = -2;
			last->next = NULL;
		}

		/* Check if we're in the last node of the level */
		if (examining->next == level) {
			steps--;
			level = last;
		}

		/* Check if the queue is empty */
		if (examining == last) {
			if (output != 'U')
				output = '0';
			break;
		}

		/* Move to the next node */
		tmp = examining;
		examining = examining->next;
		if (tmp->mine->count-- == 0) {
			free(tmp->mine->tape);
			free(tmp->mine);
		}
		free(tmp);
		stato = examining->state;
		lunghezza = examining->input_len;
		testina = examining->head;
	}
	if (!steps)
		output = 'U';

	/* Free the remaining queue */
	while (examining != last) {
		tmp = examining;
		examining = examining->next;
		/*if (tmp->mine->count-- == 0) {
			free(tmp->mine->tape);
			free(tmp->mine);
		}*/
		free(tmp);
	}
	/* Free strings */
	for (i = 0; i < upper; i++) {
		if (stringhe[i]) free(stringhe[i]);
	}
	free(stringhe);

	return EXIT_SUCCESS;
}

int initializer(void) {
	char *input;
	char read, write, move;
	int init_state, end_state, len;
	int i, j, k;

	/* Allocate acceptance state list */
	acc_max = 5;
	acceptance_state = malloc(acc_max * sizeof(int));
	if (!acceptance_state) {
		fprintf(stderr, "Could not allocate acceptance states");
		return EXIT_FAILURE;
	}

	/* Allocate head of list */
	max_states = 1;
	states = malloc(sizeof(int **));
	if (!states) {
		fprintf(stderr, "Could not allocate states\n");
		return EXIT_FAILURE;
	}
	states[0] = malloc(sizeof(int *));
	if (!states[0]) {
		fprintf(stderr, "Could not allocate the first state\n");
		return EXIT_FAILURE;
	}
	states[0][0] = calloc(4, sizeof(int));
	if (!states[0][0]) {
		fprintf(stderr, "Could not allocate first column\n");
	}

	/* Discard "tr" */
	while (reader(stdin, &input, 3))
		return EXIT_FAILURE;
	free(input);

	/* Read transition till "acc" appears */
	while (reader(stdin, &input, 12))
		return EXIT_FAILURE;
	while (input[0] != 'a') {
		if (sscanf(input, "%d %c %c %c %d", &init_state, &read, &write, &move, &end_state) != 5) {
			fprintf(stderr, "Wrong input format\n");
			return EXIT_FAILURE;
		}
		free(input);

		/* If a state with the same number does not exists, create it */
		while (init_state >= max_states) {
			states = realloc(states, (max_states + 1) * sizeof(int **));
			if (!states) {
				fprintf(stderr, "Could not reallocate states\n");
				return EXIT_FAILURE;
			}
			states[max_states] = malloc(sizeof(int *));
			if (!states[max_states]) {
				fprintf(stderr, "Could not allocate state %d\n", max_states);
				return EXIT_FAILURE;
			}
			states[max_states][0] = calloc(4, sizeof(int));
			if (!states[max_states][0]) {
				fprintf(stderr, "Could not allocate first column of %d\n", max_states);
				return EXIT_FAILURE;
			}
			++max_states;
		}

		/* Set state's attributes */
		i = 0;
		while (states[init_state][i][0] != read && states[init_state][i][0] != 0)
			i++;
		if (states[init_state][i][0] == 0) {
			states[init_state] = realloc(states[init_state], (i + 2) * sizeof(int **));
			if (!states[init_state]) {
				fprintf(stderr, "Could not reallocate state %d\n", init_state);
				return EXIT_FAILURE;
			}
			states[init_state][i + 1] = calloc(4, sizeof(int));
			if (!states[init_state][i + 1]) {
				fprintf(stderr, "Could not allocate column %d for state %d\n", i, init_state);
				return EXIT_FAILURE;
			}
		}

		/* Find the first free spot */
		j = 0;
		states[init_state][i][j] = read;
		++j;
		while (states[init_state][i][j] != 0)
			j += 3;

		states[init_state][i][j] = write;
		++j;
		if (move == 'R')
			states[init_state][i][j] = 1;
		else if (move == 'L')
			states[init_state][i][j] = -1;
		else if (move == 'S')
			states[init_state][i][j] = 0;
		++j;
		states[init_state][i][j] = end_state;
		++j;
		states[init_state][i] =
			realloc(states[init_state][i], (j + 3) * sizeof(int));
		if (!states[init_state][i]) {
			fprintf(stderr, "Could not reallocate transition for state %d\n",
				init_state);
			return EXIT_FAILURE;
		}
		for (k = 0; k < 3; k++) {
			states[init_state][i][j + k] = 0;
		}

		/* Loop over */
		while (reader(stdin, &input, 12)) {
			free(input);
			return EXIT_FAILURE;
		}
	}
	free(input);

	/* Read acceptance states till "max" appears */
	len = 0;
	while (reader(stdin, &input, 12))
		return EXIT_FAILURE;
	while (input[0] != 'm') {
		if (sscanf(input, "%d", &(acceptance_state[len])) != 1) {
			fprintf(stderr, "Wrong acceptance state\n");
			return EXIT_FAILURE;
		}
		free(input);

		/* If a state with the same number does not exists, create it */
		while (acceptance_state[len] >= max_states) {
			states = realloc(states, (max_states + 1) * sizeof(int **));
			if (!states) {
				fprintf(stderr, "Could not reallocate states\n");
				return EXIT_FAILURE;
			}
			states[max_states] = malloc(sizeof(int *));
			if (!states[max_states]) {
				fprintf(stderr, "Could not allocate the first state\n");
				return EXIT_FAILURE;
			}
			states[max_states][0] = calloc(4, sizeof(int));
			if (!states[max_states][0]) {
				fprintf(stderr, "Could not allocate first column\n");
				return EXIT_FAILURE;
			}
			max_states++;
		}

		len++;

		/* Check if there is no place left for acceptance states */
		if (len >= acc_max) {
			acceptance_state =
				realloc(acceptance_state, (acc_max + 5) * sizeof(int));
			if (!acceptance_state) {
				fprintf(stderr, "Could not reallocate acceptance state list\n");
				return EXIT_FAILURE;
			}
			acc_max += 5;
		}

		/* Loop over */
		while (reader(stdin, &input, 4)) {
			free(input);
			return EXIT_FAILURE;
		}
	}
	free(input);

	/* acc_len is the last recorded acceptance state, it's the length of the array */
	acc_max = len;

	/* Read in max runs */
	while (reader(stdin, &input, 8))
		return EXIT_FAILURE;
	if (sscanf(input, "%d", &max_runs) != 1)
		return EXIT_FAILURE;
	free(input);

	/* Discard "run" */
	while (reader(stdin, &input, 4))
		return EXIT_FAILURE;
	free(input);

	return EXIT_SUCCESS;
}

void quicker(void) {
	int i;

	for (i = 0; i < max_states; i++) {

		/* If a state is empty, skip it */
		if (states[i][0][0] == 0)
			continue;

		/* If a state has only a single transition and returns to itself, we have a loop */
		else if (states[i][1][0] == 0 && states[i][0][4] == 0 && states[i][0][3] == i) {
			states[i][0][3] = -1;
		}

	}

}

int main(void) {
	char *input, *string;
	int code, input_len, testina;
	int i, j;
	char *blanks = "__________";

	if (initializer() == EXIT_FAILURE) {
		puts("\t#########################\n\t######## FAILURE "
			"########\n\t#########################");
		return 0;
	}

	quicker();

	while (1) {
		while ((code = reader(stdin, &input, 16))) {
			if (code == 2) {
				/* I'm a one step closer to the edge, I'm about to... */ break;
			}
		}
		if (code == 2 && !strlen(input)) {
			free(input);
			break;
		}

		/* Set the starting parameters for every input */
		/* Concatenate blanks, input, and other blanks */
		string = malloc((2 * strlen(blanks) + 1 + strlen(input)) * sizeof(char));
		string[0] = '\0';
		strncat(string, blanks, strlen(blanks));
		strncat(string, input, strlen(input));
		strncat(string, blanks, strlen(blanks));
		free(input);
		input = string;
		input_len = strlen(input);

		output = 'S'; // Dummy initial output

		testina = strlen(blanks); // Place the header on the first letter of input,
								  // after all the blanks

		/* ####### BEGIN ####### */

		step(input, testina, input_len, 0, max_runs);

		/* ####### END ####### */

		printf("%c\n", output);

		free(input);

		/* Code 2 means EOF, so end of input */
		if (code == 2)
			break;
	}

	/* Clean exit */
	for (i = 0; i < max_states; i++) {
		for (j = 0; states[i][j][0] != 0; j++) {
			free(states[i][j]);
		}
		free(states[i][j]);
		free(states[i]);
	}
	free(states);

	free(acceptance_state);

	return 0;
}