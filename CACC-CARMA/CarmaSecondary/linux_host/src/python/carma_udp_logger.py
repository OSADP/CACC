
import socket, time, struct
from datetime import datetime

UDP_IP = "0.0.0.0"
UDP_PORT = 5002

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.settimeout(0.1)
sock.bind((UDP_IP, UDP_PORT))



log_filename = datetime.now().strftime('logs/carma_data_log_%d-%m-%Y-%H-%M-%S.csv');

format_string = "<ddBdBdBdBdfdfdfdfdBdfdfdddddddddfdfdfdddddddfdfdfdfdfdfdfdfdfdfdfdfdfdfdIdB"
PATH_string = "74d"
fmt = format_string+PATH_string

header_string = "msg_ts, utc_time_s, utc_time_s_ts, radar_status, radar_status_ts, acc_status, acc_status_ts, path_ctrl_status, path_ctrl_status_ts, thrtl_ovr_status, thrtl_ovr_status_ts, acc_set_spd, acc_set_spd_ts, veh_accel, veh_accel_ts, veh_spd, veh_spd_ts, str_wheel_ang, str_wheel_ang_ts, brk_lght, brk_lght_ts, brake_pdl, brake_pdl_ts, thrtl_pdl, thrtl_pdl_ts, lat, lat_ts, lon, lon_ts, alt, alt_ts, global_roll, global_roll_ts, global_pitch, global_pitch_ts, global_yaw, global_yaw_ts, local_north, local_north_ts, local_east, local_east_ts, local_down, local_down_ts, local_roll, local_roll_ts, local_pitch, local_pitch_ts, local_yaw, local_yaw_ts, horiz_pos_accy, horiz_pos_accy_ts, vert_pos_accy, vert_pos_accy_ts, fwd_vel, fwd_vel_ts, right_vel, right_vel_ts, down_vel, down_vel_ts, vel_accy, vel_accy_ts, fwd_accel, fwd_accel_ts, right_accel, right_accel_ts, down_accel, down_accel_ts, dst_to_preceed, dst_to_preceed_ts, spd_of_preceed, spd_of_preceed_ts, desired_headway, desired_headway_ts, requested_cacc_state, requested_cacc_state_ts, sec_mark, grp_id, grp_size, grp_mode, grp_man_des, grp_man_id, veh_id, cut_in_flag, veh_pos_in_grp, veh_fault_id, veh_man_des, veh_man_id, acc_cacc_sw, v, a, dist_to_pre, path_radar_status, dsrc_status, brake_pedal_deflect, throttle_pos_percent, set_v, t_gap_set, v_des, torq_des, rel_v_to_pre, rel_v_to_lead, t_gap_des_to_lead, d_gap_to_pre_est, d_gap_to_lead_est, pre_veh_v_ref, pre_veh_a, pre_veh_v, pre_veh_d_gap_ref, pre_veh_radar_dist, pre_veh_acc_cacc_sw, pre_veh_fault_mode, pre_veh_man_id, pre_veh_brk_pedal_deflection, pre_veh_v_des, pre_veh_torq_des, lead_veh_ref_v, lead_veh_a, lead_veh_v, lead_veh_d_gap_ref, lead_veh_radar_dist, lead_veh_acc_cacc_sw, lead_veh_fault_mode, lead_veh_man_id, lead_veh_brk_pedal_deflection, lead_veh_v_des, lead_veh_rotq_des, veh_1_pos, veh_1_lane_id, veh_2_pos, veh_2_lane_id, veh_3_pos, veh_3_lane_id, veh_4_pos, veh_4_lane_id, veh_5_pos, veh_5_lane_id, path_local_east, path_local_north, path_local_down, yaw, pitch, roll, yaw_rate, user_def_1, user_def_2, user_def_3, user_def_4, user_def_5\n"

with open(log_filename, 'w') as f:
    f.write(header_string)

while True:
    data = None
    try:
        data, addr = sock.recvfrom(1256) # buffer size is 1024 bytes
    except socket.timeout as e:
        pass # ignore timeouts
    if data != None:
        # print("Received message from " +str(addr)) # + ":", data
        try:
            # unpack all the data elements
            carma_data =                    struct.unpack(fmt, data) #[:struct.calcsize(fmt)])
            msg_ts =                        carma_data[0]
            radar_status =                  (carma_data[1], carma_data[2])
            acc_status =                    (carma_data[3], carma_data[4])
            path_ctrl_status =              (carma_data[5], carma_data[6])
            thrtl_ovr_status =              (carma_data[7], carma_data[8])
            acc_set_spd =                   (carma_data[9], carma_data[0])
            veh_accel =                     (carma_data[11], carma_data[12])
            veh_spd =                       (carma_data[13], carma_data[14])
            str_wheel_ang =                 (carma_data[15], carma_data[16])
            brk_lght =                      (carma_data[17], carma_data[18])
            brake_pdl =                     (carma_data[19], carma_data[10])
            thrtl_pdl =                     (carma_data[21], carma_data[22])
            utc_time =                      (carma_data[23], carma_data[24])
            lat =                           (carma_data[25], carma_data[26])
            lon =                           (carma_data[27], carma_data[28])
            alt =                           (carma_data[29], carma_data[20])
            global_roll =                   (carma_data[31], carma_data[32])
            global_pitch =                  (carma_data[33], carma_data[34])
            global_yaw =                    (carma_data[35], carma_data[36])
            local_north =                   (carma_data[37], carma_data[38])
            local_east =                    (carma_data[39], carma_data[30])
            local_down =                    (carma_data[41], carma_data[42])
            local_roll =                    (carma_data[43], carma_data[44])
            local_pitch =                   (carma_data[45], carma_data[46])
            local_yaw =                     (carma_data[47], carma_data[48])
            horiz_pos_accy =                (carma_data[49], carma_data[40])
            vert_pos_accy =                 (carma_data[51], carma_data[52])
            fwd_vel =                       (carma_data[53], carma_data[54])
            right_vel =                     (carma_data[55], carma_data[56])
            down_vel =                      (carma_data[57], carma_data[58])
            vel_accy =                      (carma_data[59], carma_data[50])
            fwd_accel =                     (carma_data[61], carma_data[62])
            right_accel =                   (carma_data[63], carma_data[64])
            down_accel =                    (carma_data[65], carma_data[66])
            dst_to_preceed =                (carma_data[67], carma_data[68])
            spd_of_preceed =                (carma_data[69], carma_data[60])
            desired_headway =               (carma_data[71], carma_data[72])
            requested_cacc_state =          (carma_data[73], carma_data[74])    
            sec_mark =                      carma_data[75]
            grp_id =                        carma_data[76]
            grp_size =                      carma_data[77]
            grp_mode =                      carma_data[78]
            grp_man_des =                   carma_data[79]
            grp_man_id =                    carma_data[80]
            veh_id =                        carma_data[81]
            cut_in_flag =                   carma_data[82]
            veh_pos_in_grp =                carma_data[83]
            veh_fault_id =                  carma_data[84]
            veh_man_des =                   carma_data[85]
            veh_man_id =                    carma_data[86]
            acc_cacc_sw =                   carma_data[87]
            v =                             carma_data[88]
            a =                             carma_data[89]
            dist_to_pre =                   carma_data[90]
            path_radar_status =                  carma_data[91]
            dsrc_status =                   carma_data[92]
            brake_pedal_deflect =           carma_data[93]
            throttle_pos_percent =          carma_data[94]
            set_v =                         carma_data[95]
            t_gap_set =                     carma_data[96]
            v_des =                         carma_data[97]
            torq_des =                      carma_data[98]
            rel_v_to_pre =                  carma_data[99]
            rel_v_to_lead =                 carma_data[100]
            t_gap_des_to_lead =             carma_data[101]
            d_gap_to_pre_est =              carma_data[102]
            d_gap_to_lead_est =             carma_data[103]
            pre_veh_v_ref =                 carma_data[104]
            pre_veh_a =                     carma_data[105]
            pre_veh_v =                     carma_data[106]
            pre_veh_d_gap_ref =             carma_data[107]
            pre_veh_radar_dist =            carma_data[108]
            pre_veh_acc_cacc_sw =           carma_data[109]
            pre_veh_fault_mode =            carma_data[110]
            pre_veh_man_id =                carma_data[111]
            pre_veh_brk_pedal_deflection =  carma_data[112]
            pre_veh_v_des =                 carma_data[113]
            pre_veh_torq_des =              carma_data[114]
            lead_veh_ref_v =                carma_data[115]
            lead_veh_a =                    carma_data[116]
            lead_veh_v =                    carma_data[117]
            lead_veh_d_gap_ref =            carma_data[118]
            lead_veh_radar_dist =           carma_data[119]
            lead_veh_acc_cacc_sw =          carma_data[120]
            lead_veh_fault_mode =           carma_data[121]
            lead_veh_man_id =               carma_data[122]
            lead_veh_brk_pedal_deflection = carma_data[123]
            lead_veh_v_des =                carma_data[124]
            lead_veh_rotq_des =             carma_data[125]
            veh_1_pos =                     carma_data[126]
            veh_1_lane_id =                 carma_data[127]
            veh_2_pos =                     carma_data[128]
            veh_2_lane_id =                 carma_data[129]
            veh_3_pos =                     carma_data[130]
            veh_3_lane_id =                 carma_data[131]
            veh_4_pos =                     carma_data[132]
            veh_4_lane_id =                 carma_data[133]
            veh_5_pos =                     carma_data[134]
            veh_5_lane_id =                 carma_data[135]
            path_local_east =                    carma_data[136]
            path_local_north =                   carma_data[137]
            path_local_down =                    carma_data[138]
            yaw =                           carma_data[139]
            pitch =                         carma_data[140]
            roll =                          carma_data[141]
            yaw_rate =                      carma_data[142]
            max_braking_exceeded =          carma_data[142]
            user_def_1 =                    carma_data[143]
            user_def_2 =                    carma_data[144]
            user_def_3 =                    carma_data[145]
            user_def_4 =                    carma_data[146]
            user_def_5 =                    carma_data[147]
        except Exception as e:
            print("Exception trying to unpack data: "+str(e))
            print("data size: "+str(len(data)))
            continue

        # Write data out to log file
        with open(log_filename, 'a') as f:
            log_string = "%.6f, %.6f, %.3f, %1i, %.3f, %1i, %.3f, %1i, %.3f, %1i, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %1i, %.3f, %.2f, %.3f, %.2f, %.3f, %.8f, %.3f, %.8f, %.3f, %.6f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.6f, %.3f, %.6f, %.3f, %.6f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.6f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %1i, %.3f, %1i, %.3f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f\n" % (msg_ts, utc_time[1], utc_time[0], radar_status[1], radar_status[0], acc_status[1], acc_status[0], path_ctrl_status[1], path_ctrl_status[0], thrtl_ovr_status[1], thrtl_ovr_status[0], acc_set_spd[1], acc_set_spd[0], veh_accel[1], veh_accel[0], veh_spd[1], veh_spd[0], str_wheel_ang[1], str_wheel_ang[0], brk_lght[1], brk_lght[0], brake_pdl[1], brake_pdl[0], thrtl_pdl[1], thrtl_pdl[0], lat[1], lat[0], lon[1], lon[0], alt[1], alt[0], global_roll[1], global_roll[0], global_pitch[1], global_pitch[0], global_yaw[1], global_yaw[0], local_north[1], local_north[0], local_east[1], local_east[0], local_down[1], local_down[0], local_roll[1], local_roll[0], local_pitch[1], local_pitch[0], local_yaw[1], local_yaw[0], horiz_pos_accy[1], horiz_pos_accy[0], vert_pos_accy[1], vert_pos_accy[0], fwd_vel[1], fwd_vel[0], right_vel[1], right_vel[0], down_vel[1], down_vel[0], vel_accy[1], vel_accy[0], fwd_accel[1], fwd_accel[0], right_accel[1], right_accel[0], down_accel[1], down_accel[0], dst_to_preceed[1], dst_to_preceed[0], spd_of_preceed[1], spd_of_preceed[0], desired_headway[1], desired_headway[0], requested_cacc_state[1], requested_cacc_state[0], sec_mark, grp_id, grp_size, grp_mode, grp_man_des, grp_man_id, veh_id, cut_in_flag, veh_pos_in_grp, veh_fault_id, veh_man_des, veh_man_id, acc_cacc_sw, v, a, dist_to_pre, path_radar_status, dsrc_status, brake_pedal_deflect, throttle_pos_percent, set_v, t_gap_set, v_des, torq_des, rel_v_to_pre, rel_v_to_lead, t_gap_des_to_lead, d_gap_to_pre_est, d_gap_to_lead_est, pre_veh_v_ref, pre_veh_a, pre_veh_v, pre_veh_d_gap_ref, pre_veh_radar_dist, pre_veh_acc_cacc_sw, pre_veh_fault_mode, pre_veh_man_id, pre_veh_brk_pedal_deflection, pre_veh_v_des, pre_veh_torq_des, lead_veh_ref_v, lead_veh_a, lead_veh_v, lead_veh_d_gap_ref, lead_veh_radar_dist, lead_veh_acc_cacc_sw, lead_veh_fault_mode, lead_veh_man_id, lead_veh_brk_pedal_deflection, lead_veh_v_des, lead_veh_rotq_des, veh_1_pos, veh_1_lane_id, veh_2_pos, veh_2_lane_id, veh_3_pos, veh_3_lane_id, veh_4_pos, veh_4_lane_id, veh_5_pos, veh_5_lane_id, path_local_east, path_local_north, path_local_down, yaw, pitch, roll, yaw_rate, max_braking_exceeded, user_def_1, user_def_2, user_def_3, user_def_4, user_def_5)
            f.write(log_string)
