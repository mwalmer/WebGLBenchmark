void setOrthographicOffCenterRH(float* matrix, float width, float height)
{
    memset(matrix, 0, 16 * sizeof(float));

    //  0  1  2  3
    //  4  5  6  7
    //  8  9 10 11
    // 12 13 14 15
    matrix[0]  = 2.0f / width;
    matrix[3]  = -1.0f;
    matrix[5]  = 2.0f / -height;
    matrix[7]  = 1.0f;
    matrix[10] = -0.5f;
    matrix[11] = 0.5f;
    matrix[15] = 1.0f;
}