#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <sys/sysmacros.h>
#include "arg_parse.h"
#include "target.h"

//rule and dependency structure
struct rulesAndDep {
	rdList next;
	char* line;
};

//target structure which contains name of target, rules, dependencies, and pointer to next node
struct targets {
	targetList next;
	char* name;
	rdList rule;
	rdList dependencies;
};

//this function creates a target node and adds dependencies if there are any
targetList createTargets(targetList* head, char* line) {
	int numOfArgs = 0;
	char** newTarget = arg_parse(line, &numOfArgs);
	targetList newTargetList = addTargets(head, *newTarget);

	if (numOfArgs != 1) {
		for (int i = 1; i < numOfArgs; i++) {
			addDependencies(newTarget[i], newTargetList);
		}
	}
	return newTargetList;
}

//this function adds a new target to the end of the targetList
targetList addTargets(targetList* current, char* targetLine) {
	char* temp = strdup(targetLine);
	tNode* headNewNode = malloc(sizeof(tNode));
	if (headNewNode == NULL) {
		fprintf(stderr, "Failed to create new node!");
	} else {
		headNewNode->next = NULL;
		headNewNode->name = temp;
		headNewNode->rule = NULL;
		headNewNode->dependencies = NULL;
	}

	while (*current != NULL) {
		current = &((*current)->next);
	}
	*current = headNewNode;
	return headNewNode;
}

//this function adds rules to the rule and dependency structure
void addRules(char *ruleLine, targetList currentTarget) {
	rdList node = NULL;
	rdNode* headStringNode = malloc(sizeof(rdNode));
	headStringNode->line = strdup(ruleLine);
	headStringNode->next = NULL;
	node = headStringNode;
	rdList *current = &currentTarget->rule;

	while (*current != NULL) {
		current = &((*current)->next);
	}
	*current = node;
}

//this function adds dependencies into the rule and dependency structure
//similar to addRules function
void addDependencies(char *dependencyLine, targetList currentTarget) {
	rdList node = NULL;
	rdNode* headStringNode = malloc(sizeof(rdNode));
	headStringNode->line = strdup(dependencyLine);
	headStringNode->next = NULL;
	node = headStringNode;
	rdList *current = &currentTarget->dependencies;

	while (*current != NULL) {
		current = &((*current)->next);
	}
	*current = node;
}

//this function will search through targetList names and
//find a match if an identical match is found
targetList targetMatch(targetList current, char *name) {
	while (current != NULL && 0 != strcmp(current->name, name)) {
		current = current->next;
	}
	return current;
}

//this function decides whether or not the rules for a dependency should execute or not
//based on if the file exists or not and the dependencies are newer than the target
bool shouldExecute(targetList targetMatchHead) {
	bool executeThisRule = false;
	int statRetVal;
	struct stat forTargetsStruct;
	struct stat forDependenciesStruct;
	stat(targetMatchHead->name, &forTargetsStruct);
	rdList newDependencyList = targetMatchHead->dependencies;

	if (newDependencyList == NULL) {
		executeThisRule = true;
	} else {
		if (access(targetMatchHead->name, F_OK) == -1) {
			executeThisRule = true;
		}
		while (newDependencyList != NULL) {
			statRetVal = stat(newDependencyList->line, &forDependenciesStruct);
			if (statRetVal == -1) {
				executeThisRule = true;
			} else if (forDependenciesStruct.st_mtime > forTargetsStruct.st_mtime) {
				executeThisRule = true;
			}
			newDependencyList = newDependencyList->next;
		}

	}
	return executeThisRule;
}

//with a target name this function will try to find matching target and
//execute the rules and dependencies for it recursively
//using the for_each logic in class
void executeRule(funcPointerToProcesline executeWithProcessLine,
		targetList* headTarget, char* targetname) {
	targetList targetMatchHead = targetMatch(*headTarget, strdup(targetname));

	if (targetMatchHead != NULL) {
		rdNode* currentDep = targetMatchHead->dependencies;
		while (currentDep != NULL) {
			executeRule(executeWithProcessLine, headTarget, currentDep->line);
			currentDep = currentDep->next;
		}

		if (shouldExecute(targetMatchHead) == true) {
			rdNode* currentRule = targetMatchHead->rule;
			while (currentRule != NULL) {
				executeWithProcessLine(currentRule->line);
				currentRule = currentRule->next;
			}
		}


	}

}