import numpy as np

def writeTo3ColumnCSVFile(param, expected, measured, filename, 
        header = 'Parameter value, Expected, Measured'):
    np.savetxt(filename, np.hstack(( 
        param.reshape((-1, 1)),\
        expected.reshape((-1, 1)),\
        measured.reshape((-1, 1)))), \
        delimiter = ',', header = header, comments = '')
