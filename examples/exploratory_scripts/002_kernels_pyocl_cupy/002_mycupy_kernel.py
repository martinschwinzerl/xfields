import numpy as np
import numpy.linalg as la

import cupy as cp

src_files = [
    '../../xfields/src_autogenerated/linear_interpolators_cuda.cu']

src_content = 'extern "C"{'
for ff in src_files:
    with open(ff, 'r') as fid:
        src_content += ('\n\n' + fid.read())
src_content += "}"

module = cp.RawModule(code=src_content)
knl_p2m_rectmesh3d= module.get_function('p2m_rectmesh3d')
knl_m2p_rectmesh3d= module.get_function('m2p_rectmesh3d')

import pickle
with open('../000_sphere/picsphere.pkl', 'rb') as fid:
    ddd = pickle.load(fid)

fmap = ddd['fmap']
x0 = fmap.x_grid[0]
y0 = fmap.y_grid[0]
z0 = fmap.z_grid[0]

dx = fmap.dx
dy = fmap.dy
dz = fmap.dz

nx = fmap.nx
ny = fmap.ny
nz = fmap.nz


pos_in_buffer_of_maps_to_interp = []
mapsize = fmap.nx*fmap.ny*fmap.nz
pos_in_buffer_of_maps_to_interp.append(0*mapsize)
pos_in_buffer_of_maps_to_interp.append(1*mapsize)
pos_in_buffer_of_maps_to_interp.append(2*mapsize)
pos_in_buffer_of_maps_to_interp.append(3*mapsize)
pos_in_buffer_of_maps_to_interp.append(4*mapsize)

nmaps_to_interp = len(pos_in_buffer_of_maps_to_interp)

x_dev = cp.array(ddd['x_test'])
y_dev = cp.array(ddd['y_test'])
z_dev = cp.array(ddd['z_test'])
n_particles = len(x_dev)
dev_offsets = cp.array(np.array(pos_in_buffer_of_maps_to_interp,
    dtype=np.int32))
dev_maps_buff = cp.array(fmap._maps_buffer)
dev_out_buff = cp.array(np.zeros(nmaps_to_interp*n_particles))
n_threads = n_particles

block_size = 256
grid_size = int(np.ceil(n_particles/block_size))

knl_m2p_rectmesh3d((grid_size, ), (block_size, ), (
        np.int32(n_particles),
        x_dev.data, y_dev.data, z_dev.data,
        x0, y0, z0, dx, dy, dz,
        np.int32(nx), np.int32(ny), np.int32(nz),
        np.int32(nmaps_to_interp),
        dev_offsets.data, dev_maps_buff.data,
        dev_out_buff.data))

# Test p2m
n_gen = 1000000
x_gen_dev = cp.array(
        np.zeros([n_gen], dtype=np.float64)+fmap.x_grid[10])
y_gen_dev = cp.array(
        np.zeros([n_gen], dtype=np.float64)+fmap.y_grid[10])
z_gen_dev = cp.array(
        np.zeros([n_gen], dtype=np.float64)+fmap.z_grid[10])
part_weights_dev = cp.array(
        np.zeros([n_gen], dtype=np.float64)+1.)
maps_buff = cp.array(0*fmap._maps_buffer)
dev_rho = maps_buff[:,:,:,1]

block_size = 256
grid_size = int(np.ceil(n_gen/block_size))

import time
t1 = time.time()
knl_p2m_rectmesh3d((grid_size, ), (block_size, ), (
        np.int32(n_gen),
        x_gen_dev.data, y_gen_dev.data, z_gen_dev.data,
        part_weights_dev.data,
        x0, y0, z0, dx, dy, dz,
        np.int32(nx), np.int32(ny), np.int32(nz),
        dev_rho.data))
a = dev_rho[10,10,10]
t2 = time.time()
print(f't = {t2-t1:.2e}')
