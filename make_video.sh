# ffmpeg -s 400x400 -pattern_type glob -i './snapshots/screens/170325_195355_plot_top_*.png' -crf 25 170325_195355_plot_top.mp4
# ffmpeg -s 400x400 -pattern_type glob -i './snapshots/screens/170325_195355_plot0_*.png' -crf 25 170325_195355_plot0.mp4
# ffmpeg -s 400x400 -pattern_type glob -i './snapshots/screens/170325_195355_plot_ortho_*.png' -crf 25 170325_195355_plot_ortho.mp4
ffmpeg -pattern_type glob -i './snapshots/screens/screen*.png' -crf 25 screen.mp4
