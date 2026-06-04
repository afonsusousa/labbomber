#include "view/glyphs.h"

const xpm_row_t* get_letter_xpm(char c) {
    if (c >= 'a' && c <= 'z') c -= 32; // Convert to uppercase for matching
    switch(c) {
        case 'A': return letter_a_xpm;
        case 'B': return letter_b_xpm;
        case 'C': return letter_c_xpm;
        case 'D': return letter_d_xpm;
        case 'E': return letter_e_xpm;
        case 'F': return letter_f_xpm;
        case 'G': return letter_g_xpm;
        case 'H': return letter_h_xpm;
        case 'I': return letter_i_xpm;
        case 'J': return letter_j_xpm;
        case 'K': return letter_k_xpm;
        case 'L': return letter_l_xpm;
        case 'M': return letter_m_xpm;
        case 'N': return letter_n_xpm;
        case 'O': return letter_o_xpm;
        case 'P': return letter_p_xpm;
        case 'Q': return letter_q_xpm;
        case 'R': return letter_r_xpm;
        case 'S': return letter_s_xpm;
        case 'T': return letter_t_xpm;
        case 'U': return letter_u_xpm;
        case 'V': return letter_v_xpm;
        case 'W': return letter_w_xpm;
        case 'X': return letter_x_xpm;
        case 'Y': return letter_y_xpm;
        case 'Z': return letter_z_xpm;
        case '0': return letter_num0_xpm;
        case '1': return letter_num1_xpm;
        case '2': return letter_num2_xpm;
        case '3': return letter_num3_xpm;
        case '4': return letter_num4_xpm;
        case '5': return letter_num5_xpm;
        case '6': return letter_num6_xpm;
        case '7': return letter_num7_xpm;
        case '8': return letter_num8_xpm;
        case '9': return letter_num9_xpm;
        case '.': return letter_dot_xpm;
        case ',': return letter_comma_xpm;
        case '-': return letter_dash_xpm;
        case ' ': return letter_space_xpm;
        default: return letter_space_xpm;
    }
}
