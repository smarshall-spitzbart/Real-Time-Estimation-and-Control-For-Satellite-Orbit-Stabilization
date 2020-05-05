import matplotlib.pyplot as plt
import numpy as np
import math
import csv
import time
import serial
import scipy
from scipy.integrate import solve_ivp
from scipy.signal import cont2discrete as c2d
from scipy.linalg import solve_discrete_are as DARE

# Setup global variables

# Run-type
hardware_in_loop = False

# Radius of the Earth [m]
R = 6378000

# Mass of the Earth [kg]

M = 5.972*(10**24)

# Mass of satellite [kg]
m = 1500

# Universal gravitation constant G [m3/(s2kg)]
G = 6.673*(10**(-11))

# Orbit turn rate for linearization [rad/s]
w0 = 7.2910**-5

# Geostationary orbit radius for linearization [m]
r0 = 4.22*10**7

'''
"In space vehicles, one can find multiple
sources of disturbances, such as position or velocity measuring errors,
thruster misalignments, or even atmospheric drag"

source: https://www.sciencedirect.com/science/article/abs/pii/S0967066111001985

These things will be our process uncertainty, v(k). If we care enough to
research how much uncertainity this will correspond to numerically, we can do
that. Otherwise we can mess with the numerics.
'''

'''
How would we make a space rendevous a 2D problem? its about launch timing,
see: https://www.youtube.com/watch?v=oNXPtZDS-cg
'''

'''
Problem could be sattellite rendevous instead of just traj stabilization?
Could formulate problem as regulating the chaser sattellites orbit to be the
same as the target sattellite. Then would use "V-bar approach" to increase
radial velocity along the target orbit until proximity is reached.
'''

def discrete_linear_dyn(r0,w0):
    dt = 10
    #discretisation obtained from http://users.wpi.edu/~zli11/teaching/rbe595_2017/LectureSlide_PDF/discretization.pdf
    A = np.array([[0, 1, 0, 0],
                  [3*w0**2, 0, 0, 2*r0*w0],
                  [0, 0, 0, 1],
                  [0, -2*w0/r0, 0, 0]])


    B = np.array([[0, 0],
                  [1, 0],
                  [0, 0],
                  [0, 1/r0]])
    
    # Golden Lecutrue Note: P. 34
    C = np.array([
        [1/r0, 0, 0, 0],
        [0   , 0, 1, 0]
        ])

    # https://docs.scipy.org/doc/scipy-0.15.1/reference/generated/scipy.signal.cont2discrete.html
    Ad, Bd, Cd, Dd, dtime = c2d((A, B, C, 0), dt, method='zoh')

    return Ad, Bd

def nl_dyn_cont(t, x, u=np.zeros(2)):
    # Continuous nonlinear dynamics of sattellite system (uses 1-d nparrays)

    '''
    Define the following quantities:
    r: dist. Earth center to satellite [m]
    phi: orbit angle of the satellite [rad]
    F_rad: radial force [N]
    F_tan: tangential force [N]
    '''
    # States:
    # x1(t) = r,
    # x2(t) = r',
    # x3(t) = phi,
    # x4(t) = phi'

    # Input:
    # u1(t) = F_rad/m,
    # u2(t) = F_tan/m

    x_out = np.array([x[1],
                      x[0]*x[3]**2-G*M*x[0]**2+u[0]/m,
                      x[3],
                      ((-2*x[1]*x[3])/x[0])+u[1]/(m*x[0])])

    # testing
    # x_out = np.zeros(4)

    '''
    # add new position to path array
    path = np.append(path, np.array([x_sat, y_sat]))
    '''
    return x_out  # , path


def lin_dyn_cont(t, x, r0, u=np.zeros((2, 1))):
    # Continuous linear dynamics of sattellite system

    # ODE solver uses 1-d arrays, convert to 2-d nparrays for lin alg
    x = np.array([[x[i]] for i in range(len(x))])

    A = np.array([[0, 1, 0, 0],
                  [3*w0**2, 0, 0, 2*r0*w0],
                  [0, 0, 0, 1],
                  [0, -2*w0/r0, 0, 0]])

    B = np.array([[0, 0],
                  [1, 0],
                  [0, 0],
                  [0, 1/r0]])

    return (A @ x + B @ u).ravel()  # output 1-d array again


def main():

    if hardware_in_loop:
        # Serial interfacing

        # Intialize ode solver
        t_sim = np.linspace(0, 10, 1)  # simulate forward 10 seconds
        x0_step = [0, 0, 0, 0]
        u = np.zeros((2, 1))

        # Serial config
        ser = serial.Serial(port='COM5', baudrate=115200, parity='N')
        if ser.is_open:
            ser.close()
            ser.open()
        else:
            ser.open()

        # Send start message
        ser.write(str.encode('s'))

        # IMPLEMENT HANDLER HERE TO ENSURE SIMULATION HAPPENS AT EQUAL TIMING INTERVALS
        print('About to enter loop')
        t_start = time.time()

        while True:
            # See (https://docs.python.org/3/library/sched.html) to replace this with an event scheduler
            if (time.time() - t_start >= 1.0):
                t_start = time.time()

                # Write input command to PSOC
                ser.write(str.encode('u'))

                # Get input integers from PSOC
                s = ser.readline().decode()
                s_processed = np.array([[int(x.strip())] for x in s.split(',')])
                t, u = s_processed[0][0], s_processed[1:3]
                print('time: ', t, '\n', 'input: ', u)
                '''
                x = ser.read()          # read one byte
                s = ser.read(10)        # read up to ten bytes (timeout)
                line = ser.readline()   # read a '\n' terminated line
                '''

                # Need to figure out how to simulate 1 step only here
                '''
                # Evaluate solution for each timestep
                step_sol = solve_ivp(lin_dyn_cont,
                                     [t_sim[0], t_sim[-1:]],
                                     x0_step, method='RK45',
                                     t_eval=t_sim, args=(r0, u))

                # reset IC for next step
                x0_step = [sol for sol in step_sol.y]

                # Real time visualization (eventually tkinter GUI)
                print(step_sol.y, u)
                '''

        '''
        Psuedocode:

        while reading serial_data:

            [x,u]=received_data
            # update the satellite coordinates and the path taken data
            X_satellite,Y_satellite,path=NonLinearDynamics(x,u)
            satellite.set_data(X_satellite, Y_satellite)
            path.set_data(path[:,0], path[:,1])


            ax.relim()
            ax.autoscale_view()
            ax.axis("equal")
            fig.canvas.draw()
        '''
    else:
        # Simulations

        # Simulate continuous linearized system
        t_sim = np.linspace(0, 100000, 10000)

        # Intial conditions, remember this is deviation from equil in polar
        x0 = [0, 0, 0, 0]
        u = np.zeros((2, 1))
        sys_sol_lin = solve_ivp(lin_dyn_cont, [t_sim[0], t_sim[-1:]], x0,
                                method='RK45', t_eval=t_sim, args=(r0, u))

        # Simulate continuous nonlinear system (yes it does run, not quickly)
        '''
        t_sim = np.linspace(0, 10, 100)
        x0 = [r0, 1, w0, 1]
        sys_sol_nl = solve_ivp(nl_dyn_cont, [t_sim[0], t_sim[-1:]], x0,
                            method='RK45', t_eval=t_sim)
        '''

        # solution will be sys_sol.y with [0:3] being arrays of state
        # print('last 10 values of radius:', sys_sol.y[0][:-10])

        # Add equil trajectory back to solution
        def equil(t):
            return np.array([r0, 0, w0*t, w0])

        for i in range(len(sys_sol_lin.y)):
            sys_sol_lin.y[i] = np.array(
                [sys_sol_lin.y[i][j]
                 + equil(t_sim[j])[i] for j in range(len(t_sim))])
            
        #implement steady state LQG
        A,B = discrete_linear_dyn(r0,w0)
        R_m = np.eye(2)
        Q = np.array([[2,400,200,200],[400,200,200,200],[200,200,400,200],[200,200,200,400]])
        H = np.array([[10,0,0,0],[0,35,0,0],[0,0,97,0],[0,0,0,96]])
        V = 0.001*np.eye(4)
        W = 300*np.eye(4)
        N = len(t_sim)
        x = np.zeros((4,N))
        xhat = np.zeros((4,N))
        x[:,0] = [42200000,0,0,4.85360589*(10**(-5))]
        
        Uinf = DARE(A.T,B,Q, R_m)
        Pinf = DARE(A.T,H.T,V, W)
        Kinf = Pinf@(H.T) @ (np.linalg.inv( H@Pinf@H.T + W))
        Finf = np.linalg.inv(R_m + B.T@Uinf@B) @ B.T@Uinf@A
        
        
        for k in range(N-1):
            #x[:,k+1]=A@x[:,k]
            x[:,k+1]=A@(x[:,k]) - B@( Finf@(xhat[:,k]) )
         
            #xhat[:,k+1]=(A - B@Finf - Kinf@H@A)@(xhat[:,k]) + Kinf@H@A@(x[:,k])
            xhat[:,k+1]=sys_sol_lin.y[0][k]

        print(x[1,:])
        print(sys_sol_lin.y[1,:])
        plt.plot(x[2,:])
        plt.plot(sys_sol_lin.y[2,:])
#         print(sys_sol_lin.y[1])
#         plt.plot(x[2,:])
#         plt.plot(sys_sol_lin.y[2])
#         plt.show()
#         
#         plt.plot(x[1,:])
#         plt.plot(sys_sol_lin.y[1])
#         plt.show()
        
#         plt.plot(x[0,:])
#         plt.plot(sys_sol_lin.y[0])
#         plt.show()
        # Convert to 2D cartesian coordinates centered at earth's core
        x_sat_lin = [sys_sol_lin.y[0][i]*math.cos(sys_sol_lin.y[2][i]) for i in range(len(t_sim))]
        y_sat_lin = [sys_sol_lin.y[0][i]*math.sin(sys_sol_lin.y[2][i]) for i in range(len(t_sim))]
        x_sat_KF = [x[0][i]*math.cos(sys_sol_lin.y[2][i]) for i in range(len(t_sim))]
        y_sat_KF = [x[0][i]*math.sin(sys_sol_lin.y[2][i]) for i in range(len(t_sim))]
        
        # x_sat_nl = [sys_sol_nl.y[0][i]*np.cos(sys_sol_nl.y[2][i]) for i in range(len(t_sim))]
        # y_sat_nl = [sys_sol_nl.y[0][i]*np.sin(sys_sol_nl.y[2][i]) for i in range(len(t_sim))]

        #  Plotting
        fig, ax = plt.subplots()
        circle1 = plt.Circle((0, 0), R, color='b')
        ax.add_artist(circle1)
        ax.plot(x_sat_KF, y_sat_KF, linewidth=2, color='r')
        # plt.plot(x_sat_nl, y_sat_nl, linewidth=2)
        plt.xlabel('x')
        plt.ylabel('y')
        plt.title(r'Sattellite Path')
        plt.gca().set_aspect('equal', adjustable='box')
        # plt.legend(['Linear model', 'Nonlinear Model'], loc='best')
        plt.show()
        fig, ax = plt.subplots()
        circle1 = plt.Circle((0, 0), R, color='b')
        ax.add_artist(circle1)
        ax.plot(x_sat_lin, y_sat_lin, linewidth=2, color='r')
        plt.show()
        '''
        # Main code and simulation here
        X_satellite, Y_satellite, path = NonLinearDynamics(x, u)

        # Visualize satellite as a cross
        satellite = plt.plot(x_sat, y_sat, color='forestgreen', marker='X', markersize=3)[0]

        # Visualize earth as a blue sphere
        earth = plt.plot(0, 0, 'bo', markersize=10)

        # model the path taken
        path_history=ax.plot(path[:,0],path[:,1],'-',color='gray',linewidth=3)[0]

        # Animate the figure
        fig = plt.figure()
        ax = plt.gca()
        plt.ion()

        plt.show()
'''


# any other functions defined here
def other():
    pass


if __name__ == '__main__':
    main()



