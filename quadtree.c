#include "quadtree.h"

int rowSize = 0;
int colSize = 0;


QD_NODE *newQuadLeaf(Coordinate topL, Coordinate botR){
    QD_NODE *b = (QD_NODE*)malloc(sizeof(QD_NODE));
    b->type = QDLEAF;
    b->topLeft = topL;
    b->botRight = botR;
    
    b->node.leaf.cords = make_point(-1,-1);
    b->node.leaf.cell.hasShip = false;
    b->node.leaf.cell.shipType = -1;
    b->node.leaf.cell.wasHit = false;
    return b;
}

QD_NODE *newQuadNode(Coordinate topL, Coordinate botR){
    QD_NODE *b = (QD_NODE*)malloc(sizeof(QD_NODE));
    b->type = QDNODE;
    b->topLeft = topL;
    b->botRight = botR;
    Coordinate c1, c2;
    

    int halfX = (b->topLeft.row + b->botRight.row + 1) / 2;
    int halfY = (b->topLeft.col + b->botRight.col + 1) / 2;

    for(int i=0;i<4;i++){
        if(i == NW){
            c1 = make_point(b->topLeft.row, b->topLeft.col);
            c2 = make_point(halfX, halfY);
        }else if(i == NE){
            c1 = make_point(halfX, b->topLeft.col);
            c2 = make_point(b->botRight.row, halfY);
        }else if(i == SW){
            c1 = make_point(b->topLeft.row, halfY);
            c2 = make_point(halfX, b->botRight.col);
        }else if(i == SE){
            c1 = make_point(halfX, halfY);
            c2 = make_point(b->botRight.row, b->botRight.col);
        }
        b->node.quadrants[i] = newQuadLeaf(c1,c2);
    }
    
    return b;
}

int getQuadrant(QD_NODE *b, Coordinate p){
    int halfX = (b->topLeft.row + b->botRight.row + 1) / 2;
    int halfY = (b->topLeft.col + b->botRight.col + 1) / 2; 

    if(halfX > p.row){
        if(halfY > p.col){
            return SW;
        }else{
            return NW;
        }
    }else{
        if(halfY > p.col){
            return SE;
        }else{
            return NE;
        }
    }
}

QD_NODE *insertCords(QD_NODE *b, Coordinate p, int shipType){
    if(b == NULL){
        return b;
    }
    
    if(b->type == QDLEAF){
        if(b->node.leaf.cords.row == -1){
            b->node.leaf.cords.row = p.row;
            b->node.leaf.cords.col = p.col;
            b->node.leaf.cell.shipType = shipType;
            b->node.leaf.cell.hasShip = true;
            b->node.leaf.cell.wasHit = false;
            return b;
        }

        // We cannot subdivide it anymore cus side is 1
        if (abs(b->topLeft.row - b->botRight.row) <= 1 && 
            abs(b->topLeft.col - b->botRight.col) <= 1) { 
            return b;
        }

        QD_NODE *node = newQuadNode(b->topLeft, b->botRight);
        node = insertCords(node, b->node.leaf.cords, b->node.leaf.cell.shipType);
        node = insertCords(node, p, shipType);
        return node;
    }


    int quad = getQuadrant(b, p);
    b->node.quadrants[quad] = insertCords(b->node.quadrants[quad], p, shipType);
    
    return b;
}

QD_NODE *search(QD_NODE *b, Coordinate p){
    
    if(b == NULL)
        return NULL;

    if(b->type == QDNODE){
        int quad = getQuadrant(b, p);
        return search(b->node.quadrants[quad], p);
    }

    if(b != NULL){
        if(b->node.leaf.cords.row == p.row && b->node.leaf.cords.col == p.col){
            return b;
        }
    }
    return NULL;
}


Coordinate make_point (int x, int y){
    Coordinate c;
    c.row = x;
    c.col = y;
    return c;
}


void printTree(QD_NODE *b){
    QD_NODE *aux = NULL;
    Coordinate p;
    for(int i=0 ;i < rowSize; i++){
		if(i <= 9){
            printf(KNRM "\t %d|",i);
        }else{
            printf(KNRM "\t%d|",i);
        }

        for(int j=0; j < colSize; j++){
            p = make_point(i,j);
            aux = search(b, p);
            if(aux == NULL){
                if(j <= 9)
                    printf(KBLU "  ~  |");
                else
                    printf(KBLU "  ~  |");
            }else{
                switch(aux->node.leaf.cell.shipType){
                case GOODSHOT:
                    printf(KYEL "  X  ");
                    printf(KBLU "|");
                    break;
                case CARRIER:
                    printf(KCYN "  C  ");
                    printf(KBLU "|");
                    break;
                case BATTLESHIP:
                    printf(KCYN "  B  ");
                    printf(KBLU "|");
                    break;
                case CRUISER:
                    printf(KCYN "  R  ");
                    printf(KBLU "|");
                    break;
                case SUBMARINE:
                    printf(KCYN "  S  ");
                    printf(KBLU "|");
                    break;
                case DESTROYER:
                    printf(KCYN "  D  ");
                    printf(KBLU "|");
                    break;
                case TSHAPE:
                    printf(KCYN "  T  ");
                    printf(KBLU "|");
                    break;
                case WATERSHOT:
                    printf(KRED "  X  ");
                    printf(KBLU "|");
                    break;
                default:  
                    break;
                }
            }
        }
        printf("\n");
    }

    printf("\t   ");
	for(int k=0;k < colSize; k++){
		if(k <=9){
            printf(KNRM "  %d  |",k);
        }else{
            printf(KNRM "  %d |",k);
        }
	}
}

QD_NODE *randomPlaceShips(QD_NODE *b, int *lstOfShips){
    Ship s;
    Coordinate c;
    struct timespec ts;
    QD_NODE *aux;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((time_t)ts.tv_nsec);
    bool isNotOnBoard = true;


    for(int i=0;i<6;i++){
        switch (i)
        {
        case CARRIER:
            for(int k=0;k<lstOfShips[i];k++){
                s.length = CARRIER_SIZE;
                s.type = CARRIER;
                while(isNotOnBoard){
                    s.rotation = (rand() % 4);
                    s.start.row = (rand() % (rowSize - CARRIER_SIZE + 1));
                    s.start.col = (rand() % (colSize - CARRIER_SIZE + 1));
                    aux = setShip(b,s);
                    if(aux != NULL){
                        b = aux;
                        isNotOnBoard = false;
                    }
                }
                isNotOnBoard = true;
            }
            break;
        case BATTLESHIP:
            for(int k=0;k<lstOfShips[i];k++){
                s.length = BATTLESHIP_SIZE;
                s.type = BATTLESHIP;
                while(isNotOnBoard){
                    s.rotation = (rand() % 4);
                    s.start.row = (rand() % (rowSize - BATTLESHIP_SIZE + 1));
                    s.start.col = (rand() % (colSize - BATTLESHIP_SIZE + 1));
                    aux = setShip(b,s);
                    if(aux != NULL){
                        b = aux;
                        isNotOnBoard = false;
                    }
                }
                isNotOnBoard = true;
            }
            break;
        case CRUISER:
            for(int k=0;k<lstOfShips[i];k++){
                s.length = CRUSIER_SIZE;
                s.type = CRUISER;
                while(isNotOnBoard){
                    s.rotation = (rand() % 4);
                    s.start.row = (rand() % (rowSize - CRUSIER_SIZE + 1));
                    s.start.col = (rand() % (colSize - CRUSIER_SIZE + 1));
                    aux = setShip(b,s);
                    if(aux != NULL){
                        b = aux;
                        isNotOnBoard = false;
                    }
                }
                isNotOnBoard = true;
            }
            break;
        case SUBMARINE:
            for(int k=0;k<lstOfShips[i];k++){
                s.length = SUBMARINE_SIZE;
                s.type = SUBMARINE;
                while(isNotOnBoard){
                    s.rotation = (rand() % 4);
                    s.start.row = (rand() % (rowSize - SUBMARINE_SIZE + 1));
                    s.start.col = (rand() % (colSize - SUBMARINE_SIZE + 1));
                    aux = setShip(b,s);
                    if(aux != NULL){
                        b = aux;
                        isNotOnBoard = false;
                    }
                }
                isNotOnBoard = true;
            }
            break;
        case DESTROYER:
            for(int k=0;k<lstOfShips[i];k++){
                s.length = DESTROYER_SIZE;
                s.type = DESTROYER;
                while(isNotOnBoard){
                    s.rotation = (rand() % 4);
                    s.start.row = (rand() % (rowSize - DESTROYER_SIZE + 1));
                    s.start.col = (rand() % (colSize - DESTROYER_SIZE + 1));
                    aux = setShip(b,s);
                    if(aux != NULL){
                        b = aux;
                        isNotOnBoard = false;
                    }
                }
                isNotOnBoard = true;
            }
            break;
        case TSHAPE:
            for(int k=0;k<lstOfShips[i];k++){
                s.length = TSHAPE_SIZE;
                s.type = TSHAPE;
                while(isNotOnBoard){
                    s.rotation = (rand() % 4);
                    s.start.row = (rand() % (rowSize - TSHAPE_SIZE + 1));
                    s.start.col = (rand() % (colSize - DESTROYER_SIZE + 1));
                    aux = setShip(b,s);
                    if(aux != NULL){
                        b = aux;
                        isNotOnBoard = false;
                    }
                }
                isNotOnBoard = true;
            }
            break;
        }
    }
    return b;
}

bool isValidPos(QD_NODE *b, Ship ship){
    QD_NODE *aux;
    if(ship.start.col < 0 || ship.start.row < 0 || ship.start.row >= rowSize || ship.start.col >= colSize){
        return false;
    }

    if(ship.rotation != ROTATION_0 && ship.rotation != ROTATION_180 && ship.rotation != ROTATION_270 && ship.rotation != ROTATION_90){
        return false;
    }

    if(ship.type == TSHAPE){
        int mid;
        if(ship.start.col + (TSHAPE_SIZE - 1) >= colSize || ship.start.row + (TSHAPE_SIZE - 1) >= rowSize){
            return false;
        }

        switch (ship.rotation)
        {
        case ROTATION_0:
            for(int i=0;i < ship.length; i++){
                aux = search(b, make_point(ship.start.row, ship.start.col + i));
                if(aux != NULL){
                    return false;
                }
                if((ship.start.col + i) > colSize){
                    return false;
                }
            }
            mid = ship.start.col + 1;
            for(int i=0;i < ship.length;i++){
                aux = search(b, make_point(ship.start.row + i, mid));
                if(aux != NULL)
                    return false;
                if(ship.start.row + i > rowSize)
                    return false;
            }
            break;
        case ROTATION_90:
            for(int i=0;i<ship.length;i++){
                aux = search(b, make_point(ship.start.row - i, ship.start.col));
                if(aux != NULL)
                    return false;
                if(ship.start.row - i < 0)
                    return false;
            }
            mid = ship.start.row - 1;
            for(int i=0;i<ship.length;i++){
                aux = search(b, make_point(mid, ship.start.col - i));
                if(aux != NULL)
                    return false;
                if(ship.start.col - i < 0)
                    return false;
            }
        break;
        case ROTATION_180:
            for(int i=0;i < ship.length;i++){
                aux = search(b, make_point(ship.start.row, ship.start.col - i));
                if(aux != NULL)
                    return false;
                if(ship.start.col - i < 0)
                    return false;
            }
            mid = ship.start.col - 1;
            for(int i=0;i < ship.length; i++){
                aux = search(b, make_point(ship.start.row - i, mid));
                if(aux != NULL)
                    return false;
                if(ship.start.row - i < 0)
                    return false;
            }
            break;
        case ROTATION_270:
            for(int i=0;i < ship.length; i++){
                aux = search(b, make_point(ship.start.row + i, ship.start.col));
                if(aux != NULL){
                    return false;
                }
                if((ship.start.row + i) > rowSize){
                    return false;
                }
            }
            mid = ship.start.row + 1;
            for(int i=0;i < ship.length;i++){
                aux = search(b, make_point(mid, ship.start.col + i));
                if(aux != NULL) 
                    return false;
                if(ship.start.col + i > colSize)
                    return false;
            }
            break;
        }
    }

    switch (ship.rotation){
    case ROTATION_0:
        if(ship.start.col + ship.length > colSize){
            return false;
        }
        for(int i=0;i<ship.length;i++){
            aux = search(b, make_point(ship.start.row, ship.start.col + i));
            if(aux != NULL){
                return false;
            }
        }
        break;
    case ROTATION_180:
        if(ship.start.col - ship.length < -1)
            return false;
        for(int i=0;i<ship.length;i++){
            aux = search(b, make_point(ship.start.row, ship.start.col - i));
            if(aux != NULL)
                return false;
        }
        break;
    case ROTATION_90:
        if(ship.start.row - ship.length < -1)
            return false;
        for(int i=0;i<ship.length;i++){
            aux = search(b, make_point(ship.start.row - i, ship.start.col));
            if(aux != NULL)
                return false;
        }
        break;
    case ROTATION_270:
        if(ship.start.row + ship.length > rowSize)
            return false;
        for(int i=0;i<ship.length;i++){
            aux = search(b , make_point(ship.start.row + i, ship.start.col));
            if(aux != NULL)
                return false;
        }
        break;
    }

    return true;
}

QD_NODE *setShip(QD_NODE *b, Ship ship){
    if(!isValidPos(b,ship)){
        return NULL;
    }

    if(ship.type == TSHAPE){
        int mid;
        switch (ship.rotation)
        {
        case ROTATION_0:
            for(int i=0;i<ship.length;i++){
                b = insertCords(b, make_point(ship.start.row, ship.start.col + i), ship.type);
            }
            mid = ship.start.col + 1;
            for(int i=0;i<ship.length;i++){
                b = insertCords(b, make_point(ship.start.row + i, mid), ship.type);
            }
            break;
        case ROTATION_90:
            for(int i=0;i<ship.length;i++){
                b = insertCords(b, make_point(ship.start.row - i, ship.start.col), ship.type);
            }
            mid = ship.start.row - 1;
            for(int i=0;i<ship.length;i++){
                b = insertCords(b, make_point(mid, ship.start.col - i), ship.type);
            }
            break;
        case ROTATION_180:
            for(int i=0;i < ship.length;i++){
                b = insertCords(b, make_point(ship.start.row, ship.start.col - i), ship.type);
            }
            mid = ship.start.col - 1;
            for(int i=0;i < ship.length; i++){
                b = insertCords(b, make_point(ship.start.row - i, mid), ship.type);
            }
            break;
        case ROTATION_270:
            for(int i=0;i<ship.length;i++){
                b = insertCords(b, make_point(ship.start.row + i, ship.start.col), ship.type);
                printf("(X,Y): (%d,%d)\n", ship.start.row + i, ship.start.col);
            }
            mid = ship.start.row + 1;
            for(int i=1;i<ship.length;i++){
                b = insertCords(b, make_point(mid, ship.start.col + i), ship.type);
            }
            break;
        }
        return b;
    }


    switch (ship.rotation)
    {
    case ROTATION_0:
        for(int i=0;i<ship.length;i++){
            b = insertCords(b, make_point(ship.start.row, ship.start.col + i), ship.type);
        }
        break;
    case ROTATION_270:
        for(int j=0;j<ship.length;j++){
            b = insertCords(b, make_point(ship.start.row + j, ship.start.col), ship.type);
        }
        break;
    case ROTATION_180:
        for(int i=0;i<ship.length;i++){
            b = insertCords(b, make_point(ship.start.row, ship.start.col - i), ship.type);
        }
        break;
    case ROTATION_90:
        for(int j=0;j<ship.length;j++){
            b = insertCords(b, make_point(ship.start.row - j, ship.start.col), ship.type);
        }
        break;
    }
    return b;
}

void setup(int row, int col){
    rowSize = row;
    colSize = col;
}

QD_NODE *manualPlaceShips(QD_NODE *b, int *lstOfShips){
    Ship s;
    Coordinate c;
    QD_NODE *aux;
    int r,col,rot;
	for(int i=0 ; i < 6 ; i++){
		switch(i){
			case CARRIER:
				for(int k=0;k<lstOfShips[i];k++){
                    s.length = CARRIER_SIZE;
                    s.type = CARRIER;
                    clearScreen();
                    printAllShipsTypes();
                    printWarningMsg("Specify the initial coordinates(row,col) of Carrier ship and your rotation. (eg 5 5 90)");
                    scanf("%d %d %d",&r,&col,&rot);
                    s.start.row = r;
                    s.start.col = col;
                    s.rotation = convertRotation(rot);
                    aux = setShip(b,s);
                    while(aux == NULL){
                        clearScreen();
                        printAllShipsTypes();
                        printErrorMsg("Wrong coordinates or rotation. Pay attention to the dimensions of the board.");
                        printWarningMsg("Please specify news coordinates(row,col) of Carrier ship and your rotation. (eg 5 5 90)");
                        scanf("%d %d %d",&r,&col,&rot);
                        s.start.row = r;
                        s.start.col = col;
                        s.rotation = convertRotation(rot);
                        aux = setShip(b,s);
                    }
                    b = aux;
                printTree(b);
                }
			break;
			case BATTLESHIP:
				for(int k=0;k<lstOfShips[i];k++){
                    s.length = BATTLESHIP_SIZE;
                    s.type = BATTLESHIP;
                    printAllShipsTypes();
                    printWarningMsg("Specify the initial coordinates(row,col) of Battleship ship and your rotation. (eg 5 5 90)");
                    scanf("%d %d %d",&r,&col,&rot);
                    s.start.row = r;
                    s.start.col = col;
                    s.rotation = convertRotation(rot);
                    aux = setShip(b,s);
                    while(aux == NULL){
                        clearScreen();
                        printAllShipsTypes();
                        printErrorMsg("Wrong coordinates or rotation. Pay attention to the dimensions of the board.");
                        printWarningMsg("Please specify news coordinates(row,col) of Battleship ship and your rotation. (eg 5 5 90)");
                        scanf("%d %d %d",&r,&col,&rot);
                        s.start.row = r;
                        s.start.col = col;
                        s.rotation = convertRotation(rot);
                        aux = setShip(b,s);
                    }
                    b = aux;
                    printTree(b);
                }
			break;
			case CRUISER:
				for(int k=0;k<lstOfShips[i];k++){
                    s.length = CRUSIER_SIZE;
                    s.type = CRUISER;
                    printAllShipsTypes();
                    printWarningMsg("Specify the initial coordinates(row,col) of Cruiser ship and your rotation. (eg 5 5 90)");
                    scanf("%d %d %d",&r,&col,&rot);
                    s.start.row = r;
                    s.start.col = col;
                    s.rotation = convertRotation(rot);
                    aux = setShip(b,s);
                    while(aux == NULL){
                        clearScreen();
                        printAllShipsTypes();
                        printErrorMsg("Wrong coordinates or rotation. Pay attention to the dimensions of the board.");
                        printWarningMsg("Please specify news coordinates(row,col) of Cruiser ship and your rotation. (eg 5 5 90)");
                        scanf("%d %d %d",&r,&col,&rot);
                        s.start.row = r;
                        s.start.col = col;
                        s.rotation = convertRotation(rot);
                        aux = setShip(b,s);
                    }
                    b = aux;
                    printTree(b);
                }
			break;
			case SUBMARINE:
				for(int k=0;k<lstOfShips[i];k++){
                    s.length = SUBMARINE_SIZE;
                    s.type = SUBMARINE;
                    printWarningMsg("Specify the initial coordinates(row,col) of Submarine ship and your rotation. (eg 5 5 90)");
                    scanf("%d %d %d",&r,&col,&rot);
                    s.start.row = r;
                    s.start.col = col;
                    s.rotation = convertRotation(rot);
                    aux = setShip(b,s);
                    while(aux == NULL){
                        clearScreen();
                        printAllShipsTypes();
                        printErrorMsg("Wrong coordinates or rotation. Pay attention to the dimensions of the board.");
                        printWarningMsg("Please specify news coordinates(row,col) of Submarine ship and your rotation. (eg 5 5 90)");
                        scanf("%d %d %d",&r,&col,&rot);
                        s.start.row = r;
                        s.start.col = col;
                        s.rotation = convertRotation(rot);
                        aux = setShip(b,s);
                    }
                    b = aux;
                    printTree(b);
                }
			break;
			case DESTROYER:
				for(int k=0;k<lstOfShips[i];k++){
                    s.length = DESTROYER_SIZE;
                    s.type = DESTROYER;
                    printAllShipsTypes();
                    printWarningMsg("Specify the initial coordinates(row,col) of Destroyer ship and your rotation. (eg 5 5 90)");
                    scanf("%d %d %d",&r,&col,&rot);
                    s.start.row = r;
                    s.start.col = col;
                    s.rotation = convertRotation(rot);
                    aux = setShip(b,s);
                    while(aux == NULL){
                        clearScreen();
                        printAllShipsTypes();
                        printErrorMsg("Wrong coordinates or rotation. Pay attention to the dimensions of the board.");
                        printWarningMsg("Please specify news coordinates(row,col) of Destroyer ship and your rotation. (eg 5 5 90)");
                        scanf("%d %d %d", &r, &col, &rot);
                        s.start.row = r ;
                        s.start.col = col ;
                        s.rotation = convertRotation(rot);
                        aux = setShip(b,s);
                    }
                    b = aux;
                    printTree(b);
                }
			break;
			case TSHAPE:
        for(int k=0;k<lstOfShips[i];k++){
            s.length = TSHAPE_SIZE;
            s.type = TSHAPE;
            printAllShipsTypes();
            printWarningMsg("Specify the initial coordinates(row,col) of Tshape ship and your rotation. (eg 5 5 90)");
            scanf("%d %d %d",&r,&col,&rot);
            s.start.row = r;
            s.start.col = col;
            s.rotation = convertRotation(rot);
            aux = setShip(b,s);
            while(aux == NULL){
                clearScreen();
                printAllShipsTypes();
                printErrorMsg("Wrong coordinates or rotation. Pay attention to the dimensions of the board.");
                printWarningMsg("Please specify news coordinates(row,col) of Tshape ship and your rotation. (eg 5 5 90)");
                scanf("%d %d %d", &r, &col, &rot);
                s.start.row = r ;
                s.start.col = col ;
                s.rotation = convertRotation(rot);
                aux = setShip(b,s);
            }
            b = aux;
            printTree(b);
        }
			break;
		}
	}
    return b;
}

int convertRotation(int rot){
	if(rot == 0){
		return ROTATION_0;
	}
	if(rot == 90){
		return ROTATION_90;
	}
	if(rot == 180){
		return ROTATION_180;
	}
	if(rot == 270){
		return ROTATION_270;
	}
    return -1;
}

void getValidShot(Coordinate *c){
    while(c->row < 0 || c->col < 0 || c->row >= rowSize || c->col >= colSize){
        printErrorMsg("That is an invalid Coordinate, please respect the board edges.");
        printWarningMsg("Introduce one shot coordinates (R C):\n");
        scanf("%d%d",&c->row,&c->col);
    }
}

bool isInPreviousShots(Shots *lst, Coordinate c){
    Shots *aux;
    aux = searchShot(lst, c);
    if(aux == NULL){
        return false;
    }
    return true;
}

void fire(QD_NODE* p1board, QD_NODE* p2board, int *p1Hp, int *p2Hp){
    QD_NODE *aux;
    int turn = 1;
    Coordinate shot;
    int x, y;
    Shots *p1Shots = NULL, *p2Shots = NULL, *shotsAux = NULL;
    
    while(true){
        if(turn == 1){
            clearScreen();
            printWarningMsg("\t\t\t\t\t\t\t#### Board Player 1 ###\n\n");
            printTree(p1board);
            printWarningMsg("\n\nPlayer 1 is your turn!\n");
            printWarningMsg("Specify the coordinates of your shots:");

            for(int i=0; i<3; i++){
                printWarningMsgInt("Shot" ,i+1);
                scanf("%d%d", &shot.row, &shot.col);
                getValidShot(&shot);
                while(isInPreviousShots(p1Shots, shot)){
                    printErrorMsg("Coordinate already been used, please enter a new coordinate.");
                    printWarningMsg("Player 1 introduce one shot coordinates (R C):\n");
                    scanf("%d%d",&shot.row, &shot.col);
                    getValidShot(&shot);
                }
                aux = search(p2board, shot);
                if(aux != NULL){ //If aux != NULL then exists coords in quad tree so there is a boat
                    if(aux->node.leaf.cell.wasHit == false){ // coords was not hit yet
                        p1Shots = newShot(p1Shots, shot, true);
                        *p2Hp = *p2Hp - 1;
                        aux->node.leaf.cell.wasHit = true;
                        aux->node.leaf.cell.shipType = GOODSHOT;
                        if(*p2Hp == 0){
                            printSuccessMsg("Congratulations , Player 1 won! ");
                            printTree(p2board);
                            return;
                        }
                        printShots(p1Shots);
                    }
                }else{//Did not found that coords, so it's a watershot
                    p1Shots = newShot(p1Shots, shot, false);
                    printShots(p1Shots);
                }
            }
            turn = 2;
        }else{ // PLAYER 2 TURN
            clearScreen();
            printWarningMsg("\t\t\t\t\t\t\t#### Board Player 2 ###\n\n");
            printTree(p2board);
            printWarningMsg("\n\nPlayer 2 is your turn!\n");
            printWarningMsg("Specify the coordinates of your shots:");

            for(int i=0; i<3; i++){
                printWarningMsgInt("Shot" ,i+1);
                scanf("%d%d", &shot.row, &shot.col);
                getValidShot(&shot);
                while(isInPreviousShots(p1Shots, shot)){
                    printErrorMsg("Coordinate already been used, please enter a new coordinate.");
                    printWarningMsg("Player 1 introduce one shot coordinates (R C):\n");
                    scanf("%d%d",&shot.row, &shot.col);
                    getValidShot(&shot);
                }
                aux = search(p1board, shot);
                if(aux != NULL){ //If aux != NULL then exists coords in quad tree so there is a boat
                    if(aux->node.leaf.cell.wasHit == false){ // coords was not hit yet
                        p2Shots = newShot(p2Shots, shot, true);
                        *p1Hp = *p1Hp - 1;
                        aux->node.leaf.cell.wasHit = true;
                        aux->node.leaf.cell.shipType = GOODSHOT;
                        if(*p1Hp == 0){
                            printSuccessMsg("Congratulations , Player 1 won! ");
                            printTree(p1board);
                            return;
                        }
                        printShots(p2Shots);
                    }
                }else{//Did not found that coords, so it's a watershot
                    p2Shots = newShot(p2Shots, shot, false);
                    printShots(p2Shots);
                }
            }
            turn = 1;
        }
    }
}