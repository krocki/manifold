/*
* @Author: Kamil Rocki
* @Date:   2017-03-16 14:57:29
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-17 20:31:57
*/
void drawline ( Eigen::MatrixXf &boxline_positions, Eigen::MatrixXf &boxline_colors, size_t &m_boxlineCount,
                Eigen::Vector3f pos0, Eigen::Vector3f pos1, Eigen::Vector4f color ) {

	m_boxlineCount = 6;
	boxline_positions.resize ( 3, m_boxlineCount );
	boxline_colors.resize ( 4, m_boxlineCount );

	boxline_positions.col ( 0 ) << pos0[0], pos0[1], pos0[2];
	boxline_positions.col ( 1 ) << pos1[0], pos1[1], pos1[2];

	//crosshair
	boxline_positions.col ( 2 ) << pos0[0] - 0.001f, pos0[1], pos1[2] - 0.0001f;
	boxline_positions.col ( 3 ) << pos1[0] + 0.001f, pos1[1], pos1[2] - 0.0001f;
	boxline_positions.col ( 4 ) << pos0[0], pos0[1] - 0.001f, pos0[2] + 0.0001f;
	boxline_positions.col ( 5 ) << pos1[0], pos1[1] + 0.001f, pos0[2] + 0.0001f;

	for ( size_t i = 0; i < m_boxlineCount; i++ )
		boxline_colors.col ( i ) = color;

}

void drawbox ( Eigen::MatrixXf &boxline_positions, Eigen::MatrixXf &boxline_colors, size_t &m_boxlineCount,
               Eigen::Vector3f &pos, Eigen::Vector3f &r, Eigen::Vector4f color,  float cursor_x, float cursor_y ) {

	m_boxlineCount = 16 * 2;

	boxline_positions.resize ( 3, m_boxlineCount );
	boxline_colors.resize ( 4, m_boxlineCount );

	boxline_positions.col ( 0 ) << pos[0] - r[0], pos[1] - r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 1 ) << pos[0] - r[0], pos[1] + r[1], pos[2] - r[2] + 0.0001f; //L
	boxline_positions.col ( 2 ) << pos[0] + r[0], pos[1] - r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 3 ) << pos[0] + r[0], pos[1] + r[1], pos[2] - r[2] + 0.0001f; //R
	boxline_positions.col ( 4 ) << pos[0] - r[0], pos[1] - r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 5 ) << pos[0] + r[0], pos[1] - r[1], pos[2] - r[2] + 0.0001f; //B
	boxline_positions.col ( 6 ) << pos[0] - r[0], pos[1] + r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 7 ) << pos[0] + r[0], pos[1] + r[1], pos[2] - r[2] + 0.0001f; //T
	boxline_positions.col ( 8 ) << pos[0] - r[0], pos[1] - r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 9 ) << pos[0] - r[0], pos[1] - r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 10 ) << pos[0] + r[0], pos[1] - r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 11 ) << pos[0] + r[0], pos[1] - r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 12 ) << pos[0] - r[0], pos[1] + r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 13 ) << pos[0] - r[0], pos[1] + r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 14 ) << pos[0] + r[0], pos[1] + r[1], pos[2] - r[2] + 0.0001f;
	boxline_positions.col ( 15 ) << pos[0] + r[0], pos[1] + r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 16 ) << pos[0] - r[0], pos[1] - r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 17 ) << pos[0] - r[0], pos[1] + r[1], pos[2] + r[2] - 0.0001f; //UL
	boxline_positions.col ( 18 ) << pos[0] + r[0], pos[1] - r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 19 ) << pos[0] + r[0], pos[1] + r[1], pos[2] + r[2] - 0.0001f; //UR
	boxline_positions.col ( 20 ) << pos[0] - r[0], pos[1] - r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 21 ) << pos[0] + r[0], pos[1] - r[1], pos[2] + r[2] - 0.0001f; //UB
	boxline_positions.col ( 22 ) << pos[0] - r[0], pos[1] + r[1], pos[2] + r[2] - 0.0001f;
	boxline_positions.col ( 23 ) << pos[0] + r[0], pos[1] + r[1], pos[2] + r[2] - 0.0001f; //UT

	// dir
	boxline_positions.col ( 24 ) << pos[0] - r[0] / 2, pos[1] - r[1], pos[2] + 9 * r[2] / 10;
	boxline_positions.col ( 25 ) << pos[0] + r[0] / 2, pos[1] - r[1], pos[2] + 9 * r[2] / 10;

	//crosshair
	boxline_positions.col ( 26 ) << pos[0] + cursor_x, pos[1] + cursor_y, pos[2] - r[2] + 0.001f;
	boxline_positions.col ( 27 ) << pos[0] + cursor_x, pos[1] + cursor_y, pos[2] + r[2] - 0.001f;

	boxline_positions.col ( 28 ) << pos[0] + cursor_x - 0.001f, pos[1] + cursor_y, pos[2] - r[2] + 0.001f;
	boxline_positions.col ( 29 ) << pos[0] + cursor_x + 0.001f, pos[1] + cursor_y, pos[2] - r[2] + 0.001f;
	boxline_positions.col ( 30 ) << pos[0] + cursor_x, pos[1] + cursor_y - 0.001f, pos[2] - r[2] - 0.001f;
	boxline_positions.col ( 31 ) << pos[0] + cursor_x, pos[1] + cursor_y + 0.001f, pos[2] - r[2] - 0.001f;

	for ( size_t i = 0; i < 26; i++ )
		boxline_colors.col ( i ) = color;

	for ( size_t i = 26; i < m_boxlineCount; i++ )
		boxline_colors.col ( i ) = nanogui::Color ( 255, 0, 0, 255 );


}