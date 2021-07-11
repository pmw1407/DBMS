#include "bpt.h"
#include "file.h"
#include "buffer.h"

// MAIN

extern FILE * fp;
extern page_t* current_page;

int main( int argc, char ** argv ) {
    int bufsize;
    printf("Input Max Buffer Size : ");
    scanf("%d", &bufsize);
    
    init_db(bufsize);

    int table_id;
    char input_file[100];
    int input, range2;
    int instruction;
    char license_part;

    int64_t key;
    char value[120];

    //license_notice();
    //usage_1();  
    //usage_2();

    while(true){
        show_menu();
        scanf("%d", &instruction);

        if(instruction == 5){
            shutdown_db();
            
            break;
        }
        
        else if(instruction == 1){
            printf("Input file name : ");
            scanf("%s", input_file);
            table_id = open_table(input_file);
            printf("table id : %d\n", table_id);
        }

        else if(instruction == 2){
            printf("Input key : ");
            scanf("%ld", &key);
            printf("Input value : ");
            scanf("%s", value);

            db_insert(table_id, key, value);
            printf("insertion success\n");
            print_tree(table_id);
        }

        else if(instruction == 4){
            printf("Input key : ");
            scanf("%ld", &key);

            db_delete(table_id, key);

            print_tree(table_id);
        }

        else if(instruction == 6){
            print_tree(table_id);
        }

        else if(instruction == 7){
            printf("Insert Range start from 1 : ");
            int64_t end;
            scanf("%ld", &end);
            char a[120];
            strcpy(a, "a");
            for(int i = 1; i <= end; i++){
                db_insert(table_id, i, a);
            }
        }

        else if(instruction == 8){
            printf("Delete Range start from 1 : ");
            int64_t end;
            scanf("%ld", &end);
            for(int i = 1; i <= end; i++){
                db_delete(table_id, i);
            }
        }

        else if(instruction == 9){
            close_table(table_id);
            printf("%d table closed\n", table_id);
            // printf("Input file name to close : ");
            // scanf("%s", input_file);
            // table_id = open_table(input_file);
            // close_table(table_id);
        }

        else{
            printf("Wrong instruction input. Please try again\n");
            getchar();
            
        }
    }

    return EXIT_SUCCESS;
    
}
