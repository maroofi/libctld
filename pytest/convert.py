if __name__ == "__main__":
    data = None
    fw = open("psl_data.h", "w")
    fw.write('char * psl_data =\n')
    with open("psl.dat") as f:
        for line in f:
            line = line.strip()
            fw.write('"' + line + '\\n"' + "\n")
    fw.write('"";')
            
