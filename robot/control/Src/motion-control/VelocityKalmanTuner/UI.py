from PyQt5.QtWidgets import (QGridLayout, QMainWindow, QWidget, QVBoxLayout, QGroupBox, QLabel, QLineEdit, QPushButton,
                             QHBoxLayout, QAction, QComboBox)
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg, NavigationToolbar2QT
import matplotlib.pyplot as plt


class UIMainWindow(QMainWindow):
    """Main Window for Application"""

    def __init__(self):
        super(UIMainWindow, self).__init__()
        self.setWindowTitle("Velocity Kalman Filter Tuner")
        self.resize(self.screen().size()*2/3)
        self.central_widget = QWidget(self)
        self.master_grid_layout = QGridLayout()

        self.plot_box = QGroupBox(self.central_widget)
        self.plot_box_layout = QVBoxLayout(self.plot_box)
        self.plot_fig = plt.figure()
        self.plot_canvas = FigureCanvasQTAgg(self.plot_fig)
        self.vx_ax = self.plot_fig.add_subplot(3, 1, 1)
        self.vy_ax = self.plot_fig.add_subplot(3, 1, 2)
        self.omega_ax = self.plot_fig.add_subplot(3, 1, 3)
        self.plot_axs = [self.vx_ax, self.vy_ax, self.omega_ax]

        self.vx_ax.set_title('X velocity over Time')
        self.vx_ax.set_xlabel('t')
        self.vx_ax.set_ylabel('vx')

        self.vy_ax.set_title('Y velocity over Time')
        self.vy_ax.set_xlabel('t')
        self.vy_ax.set_ylabel('vy')

        self.omega_ax.set_title('Omega vs Time')
        self.omega_ax.set_xlabel('t')
        self.omega_ax.set_ylabel('\u03C9')  # omega character

        for ax in self.plot_axs:
            ax.spines['top'].set_visible(False)
            ax.spines['right'].set_visible(False)

        self.plot_toolbar = NavigationToolbar2QT(self.plot_canvas, self)
        self.plot_box_layout.addWidget(self.plot_canvas)
        self.plot_box_layout.addWidget(self.plot_toolbar)

        self.right_box = QGroupBox(self.central_widget, title="Gains")
        self.right_box_layout = QVBoxLayout(self.right_box)

        self.settings_box = QGroupBox(self.central_widget)
        self.settings_box_layout = QVBoxLayout(self.settings_box)
        self.filter_type_box = QGroupBox(self.central_widget)
        self.filter_type_box_layout = QHBoxLayout(self.filter_type_box)
        self.filter_type_label = QLabel("Kalman Filter Type: ")
        self.filter_type_select = QComboBox(self.central_widget)
        self.filter_type_select.addItem("Regular")
        self.filter_type_select.addItem("Steady State")
        self.filter_type_box_layout.addWidget(self.filter_type_label)
        self.filter_type_box_layout.addWidget(self.filter_type_select)
        self.filter_type_box.setFlat(True)

        self.settings_box_layout.addWidget(self.filter_type_box)
        self.settings_box.setFlat(True)

        self.Q_box = QGroupBox(self.central_widget)
        self.Q_box_layout = QGridLayout(self.Q_box)
        self.Q_label = QLabel("Q Gains")
        self.Q_box_layout.addWidget(self.Q_label, 0, 0, 1, 1)
        self.Q_textboxes = []
        for i in range(1, 4):
            row = []
            for j in range(0, 3):
                textbox = QLineEdit(self.central_widget)
                row.append(textbox)
                self.Q_box_layout.addWidget(textbox, i, j, 1, 1)
            self.Q_textboxes.append(row)
        self.Q_box.setFlat(True)

        self.R_box = QGroupBox(self.central_widget)
        self.R_box_layout = QGridLayout(self.R_box)
        self.R_label = QLabel("R Gains")
        self.R_box_layout.addWidget(self.R_label, 0, 0, 1, 1)
        self.R_textboxes = []
        for i in range(1, 6):
            row = []
            for j in range(0, 5):
                textbox = QLineEdit(self.central_widget)
                row.append(textbox)
                self.R_box_layout.addWidget(textbox, i, j, 1, 1)
            self.R_textboxes.append(row)
        self.R_box.setFlat(True)

        self.buttons_box = QGroupBox(self.central_widget)
        self.buttons_box_layout = QHBoxLayout(self.buttons_box)
        self.sim_bttn = QPushButton("Simulate")
        self.gen_bttn = QPushButton("Generate C++ Code")
        self.buttons_box_layout.addWidget(self.sim_bttn)
        self.buttons_box_layout.addWidget(self.gen_bttn)
        self.buttons_box.setFlat(True)

        self.right_box_layout.addWidget(self.filter_type_box)
        self.right_box_layout.addWidget(self.Q_box)
        self.right_box_layout.addWidget(self.R_box)
        self.right_box_layout.addWidget(self.buttons_box)

        self.master_grid_layout.addWidget(self.plot_box, 0, 0, 1, 1)
        self.master_grid_layout.addWidget(self.right_box, 0, 1, 1, 1)
        self.master_grid_layout.setColumnStretch(0, 2)
        self.master_grid_layout.setColumnStretch(1, 1)

        self.setCentralWidget(self.central_widget)
        self.central_widget.setLayout(self.master_grid_layout)

        self.menu_bar = self.menuBar()
        self.save_action = QAction('Save Gains', self)
        self.save_action.setShortcut('Ctrl+S')
        self.load_action = QAction('Import Gains', self)
        self.load_action.setShortcut('Ctrl+I')
        self.file_menu = self.menu_bar.addMenu('File')
        self.file_menu.addAction(self.save_action)
        self.file_menu.addAction(self.load_action)


    def set_plot_data(self, ts, vxs, vxs_obs, vys, vys_obs, omegas, omegas_obs):
        for ax in self.plot_axs:
            ax.lines.clear()

        self.vx_ax.plot(ts, vxs, '-', c='tab:blue')
        self.vy_ax.plot(ts, vys, '-', c='tab:blue')
        self.omega_ax.plot(ts, omegas, '-', c='tab:blue')

        self.vx_ax.plot(ts, vxs_obs, '-', c='tab:green')
        self.vy_ax.plot(ts, vys_obs, '-', c='tab:green')
        self.omega_ax.plot(ts, omegas_obs, '-', c='tab:green')

        for ax in self.plot_axs:
            ax.relim()
            ax.legend(['x_hat', 'x'])

        self.plot_canvas.draw()