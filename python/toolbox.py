import subprocess
import shlex

def prompt_command(cmd):
    """
    Run a prompt command and returns stdout, stderr and command code

    Args:
        cmd (str): The command to be executed

    Returns:
        tuple: A tuple containing:
            - str: The standard output of the command.
            - str: The standard error of the command.
            - int: The return code of the command.

    Raises:
        subprocess.CalledProcessError: If the command returns a non-zero exit code.
        Exception: If an error occurs during command execution.

    Example:
        cmd = "ls -l"
        stdout, stderr, return_code = prompt_command(cmd)

        if return_code == 0:
            print("Command executed successfully.")
            print("Standard Output:")
            print(stdout)
        else:
            print(f"Command failed with return code {return_code}.")
            print("Error output:")
            print(stderr)
    """    
    
    try:
        # remove double space and split into a list
        # cmd = ' '.join(cmd.split()).split()
        # Use shlex.split to correctly split the command into a list
        cmd = shlex.split(cmd)

        # Run the command and capture its output
 # why check=True? maybe not needed        
        # completed_process = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        completed_process = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Extract useful information
        stdout_text = completed_process.stdout
        stderr_text = completed_process.stderr
        return_code = completed_process.returncode

        # Return the useful information as a tuple
        return stdout_text, stderr_text, return_code

# Maybe not needed
    # except subprocess.CalledProcessError as e:
    #     # The command returned a non-zero exit code, indicating an error
    #     return None, e.stderr, e.returncode

    except Exception as e:
        # Other exceptions (e.g., if the command doesn't exist)
        return None, str(e), 1  # Return code 1 for generic error
 
# def file_lines(file_path, comment='#'):
#     try:
#         with open(file_path, "r") as file:
#             # Read all lines from the file and store them in a list
#             lines = file.readlines()
#             # Remove the newline character from the end of each line
#             lines = [l.strip() for l in lines]
#             # Remove commented lines
#             if comment:
#                 lines = [l for l in lines if not l.startswith(comment)]
#             return lines
#     except FileNotFoundError:
#         print(f"The file '{file_path}' was not found.")
#         return None

def file_lines(file_path, comment='#'):
    try:
        result = []
        with open(file_path, "r") as file:
            lines = file.readlines()
            for line in lines:
                items = line.split(',')
                for item in items:
                    item = item.strip()
                    if not item.startswith(comment):
                        result.append(item)
        return result
    except FileNotFoundError:
        print(f"The file '{file_path}' was not found.")
        return None

def print_markdown_table(columns, titles, decimals=-1, output=None):
    # Check if the number of columns and titles match
    if len(columns) != len(titles):
        print("Error: The number of columns and column titles must match.")
        return

    # Function to determine alignment based on the data type
    def determine_alignment(data):
        return "---:" if all(isinstance(item, (int, float)) for item in data) else "---"

    # Create a list to store the table lines
    table_lines = []

    # Determine alignments for each column
    alignments = [determine_alignment(col) for col in columns]

    # Print the table header
    header = "| " + " | ".join(titles) + " |"
    alignment = "| " + " | ".join(alignments) + " |"
    table_lines.extend([header, alignment])

    # Find the maximum number of rows in any column
    max_rows = max(len(col) for col in columns)

    # Print the table rows
    for row_index in range(max_rows):
        row_values = [col[row_index] if row_index < len(col) else "" for col in columns]
        row = "| " + " | ".join(
           map(
              lambda x: (
                 f"{x}" if isinstance(x, (int, float)) and decimals < 0 else
                 (f"{x:.{decimals}f}" if isinstance(x, (int, float)) and x >= 1 else f"{x:.{decimals}f}")
               ) if isinstance(x, (int, float)) else str(x),
               row_values
            )
         ) + " |"

        table_lines.append(row)
    # If an output file is provided, write to it; otherwise, print on the screen
    if output is not None:
        with open(output, 'w') as file:
            file.write('\n'.join(table_lines))
    else:
        for line in table_lines:
            print(line)        

def parse_listfile(list_arg):
    if list_arg.endswith('.txt'):
        with open(list_arg, 'r') as file:
            content = file.read().strip()
            
            # # Check if the content is comma-separated or one item per line (replace by below)
            # if ',' in content:
            #     items = content.split(',')
            # else:
            #     items = content.split('\n')

            # In case there are multiple lines with csv in each line
            content = content.replace(' ','')
            content = content.replace('\n',',')

            items = content.split(',')
            items = [item for item in items if not item.startswith('#')]  
    else:
        items = list_arg.split(',')
    return items

