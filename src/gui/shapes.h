/*
* @Author: Kamil Rocki
* @Date:   2017-03-16 14:57:29
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-16 15:03:00
*/

void drawbox ( Eigen::MatrixXf &boxline_positions, size_t &m_boxlineCount, Eigen::Vector3f &pos, Eigen::Vector3f &r ) {

	m_boxlineCount = 14 * 2;
	
	boxline_positions.resize ( 3, m_boxlineCount );
	
	boxline_positions.col ( 0 ) << pos[0] - r[0], pos[1] - r[1], pos[2] - r[2];
	boxline_positions.col ( 1 ) << pos[0] - r[0], pos[1] + r[1], pos[2] - r[2]; //L
	boxline_positions.col ( 2 ) << pos[0] + r[0], pos[1] - r[1], pos[2] - r[2];
	boxline_positions.col ( 3 ) << pos[0] + r[0], pos[1] + r[1], pos[2] - r[2]; //R
	boxline_positions.col ( 4 ) << pos[0] - r[0], pos[1] - r[1], pos[2] - r[2];
	boxline_positions.col ( 5 ) << pos[0] + r[0], pos[1] - r[1], pos[2] - r[2]; //B
	boxline_positions.col ( 6 ) << pos[0] - r[0], pos[1] + r[1], pos[2] - r[2];
	boxline_positions.col ( 7 ) << pos[0] + r[0], pos[1] + r[1], pos[2] - r[2]; //T
	boxline_positions.col ( 8 ) << pos[0] - r[0], pos[1] - r[1], pos[2] - r[2];
	boxline_positions.col ( 9 ) << pos[0] - r[0], pos[1] - r[1], pos[2] + r[2];
	boxline_positions.col ( 10 ) << pos[0] + r[0], pos[1] - r[1], pos[2] - r[2];
	boxline_positions.col ( 11 ) << pos[0] + r[0], pos[1] - r[1], pos[2] + r[2];
	boxline_positions.col ( 12 ) << pos[0] - r[0], pos[1] + r[1], pos[2] - r[2];
	boxline_positions.col ( 13 ) << pos[0] - r[0], pos[1] + r[1], pos[2] + r[2];
	boxline_positions.col ( 14 ) << pos[0] + r[0], pos[1] + r[1], pos[2] - r[2];
	boxline_positions.col ( 15 ) << pos[0] + r[0], pos[1] + r[1], pos[2] + r[2];
	boxline_positions.col ( 16 ) << pos[0] - r[0], pos[1] - r[1], pos[2] + r[2];
	boxline_positions.col ( 17 ) << pos[0] - r[0], pos[1] + r[1], pos[2] + r[2]; //UL
	boxline_positions.col ( 18 ) << pos[0] + r[0], pos[1] - r[1], pos[2] + r[2];
	boxline_positions.col ( 19 ) << pos[0] + r[0], pos[1] + r[1], pos[2] + r[2]; //UR
	boxline_positions.col ( 20 ) << pos[0] - r[0], pos[1] - r[1], pos[2] + r[2];
	boxline_positions.col ( 21 ) << pos[0] + r[0], pos[1] - r[1], pos[2] + r[2]; //UB
	boxline_positions.col ( 22 ) << pos[0] - r[0], pos[1] + r[1], pos[2] + r[2];
	boxline_positions.col ( 23 ) << pos[0] + r[0], pos[1] + r[1], pos[2] + r[2]; //UT
	
	// dir
	boxline_positions.col ( 24 ) << pos[0] + r[0] / 2, pos[1] - r[1], pos[2] + 3 * r[2] / 4;
	boxline_positions.col ( 25 ) << pos[0], pos[1] - r[1], pos[2] + 2 * r[2] / 4;
	boxline_positions.col ( 26 ) << pos[0], pos[1] - r[1], pos[2] + 2 *  r[2] / 4;
	boxline_positions.col ( 27 ) << pos[0] - r[0] / 2, pos[1] - r[1], pos[2] + 3 * r[2] / 4;
	
}