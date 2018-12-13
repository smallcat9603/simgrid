#!/usr/bin/ruby
# 
# 	ラック配置→FSO端末配置→見通し率計算
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

AISLE_DEPTH    = 1100 # Aisle depth (X axis) [mm]
RACK_DEPTH     = 1000 # Rack depth (X axis) [mm]
RACK_WIDTH     =  600 # Rack width (Y axis) [mm]
RACK_HEADROOM  = 1200 # Max height (Z axis) over a rack [mm]
FSO_POLE_SIZE  =  5.0 # Radius of pole [mm]
FSO_BEAM_SIZE  =  1.0 # Radius of beam [mm]
TMPDIR         = "/tmp" # If TMPDIR is local, set PARALLEL_OPT to use local CPUs
PARALLEL_OPT   = "-Jlocal -j100%" # Options for GNU Parallel used internally

$help_message = <<-EOM
Usage:
  #{$0} [Options] Floorplan NumTerminal Arrangement

Options:
  -r <NUM>	radius of an FSO terminal [mm] (default: 12.5)
  -f		overwrite existing log file
  -m		save .map file
  -w		save .pbrt file
  -s		render the whole scene
  -c <NUM>	create a checksheet for at most <NUM> terminals
  -b <NUM>	render <NUM> images by each pbrt process (default: 100)
  -a		allow violation of territory
  -n		layout only; do not render/compute the line-of-sight ratio
  --seed <NUM>	random seed
  -v     	verbose mode
  --debug	debug mode

Floorplan:
  <X>x<Y>	<X> by <Y> racks
  <N>n  	<N> racks arranged by a square number (e.g. 16=4x4)
  <N>p  	<N> racks on a square floor (X-axis round-up, e.g. 16=3x6-2)
  <N>d  	<N> racks on a square floor (X-axis round-down, e.g. 16=2x8)

NumTerminal:
  Number of FSO terminals per rack (not implemented! Must be set to 1)

Arrangement:
  <X>-<Y>	<X>:{Fix|RndY|ArcY|ArcZ}, <Y>:{Fix|RndX|ArcX|ArcZ}
  RndXYZ
  Hybrid<N>	ArcZ-ArcX for <N> rows on both sides; RndY-RndX for other rows

Example:
  #{$0} -s -c100 -r12.5 4x8 1 ArcZ-ArcX
EOM

require "rubygems"
require "parallel"
require "matrix"
require "optparse"
require "pathname"
require "pp"

def main
	$mydir  = Pathname(__FILE__).expand_path.dirname
	$myname = Pathname(__FILE__).expand_path.basename(".*")
	$warning = Hash.new
	$opts = Hash.new
	opt = OptionParser.new
	opt.on("-r NUM") do |v|
		$opts[:termsize] = v.to_f
	end
	opt.on("-f") do |v|
		$opts[:overwrite] = v
	end
	opt.on("-m") do |v|
		$opts[:save_map] = v
	end
	opt.on("-w") do |v|
		$opts[:save_pbrt] = v
	end
	opt.on("-s") do |v|
		$opts[:save_scene] = v
	end
	opt.on("-c NUM") do |v|
		$opts[:montage] = v.to_i
	end
	opt.on("-b NUM") do |v|
		$opts[:batch] = v.to_i
	end
	opt.on("-a") do |v|
		$opts[:allow_violation] = v
	end
	opt.on("-n") do |v|
		$opts[:coldrun] = v
	end
	opt.on("--seed NUM") do |v|
		$opts[:seed] = v.to_i
	end
	opt.on("-v") do |v|
		$verbose = v
	end
	opt.on("--debug") do |v|
		$debug = v
	end
	opt.parse!(ARGV)
	if ARGV.length < 3
		puts $help_message
		exit(1)
	end
	floorplan   = ARGV[0]
	termperrack = ARGV[1].to_i
	arrangement = ARGV[2]
	parameter = ARGV.join("-")
	parameter += "(#{$opts[:seed]})" if $opts[:seed]
	logfile = Pathname("#{parameter}.log")
	mapfile = Pathname("#{parameter}.map")
	pbrfile = Pathname("#{parameter}.pbrt")
	scnfile = Pathname("#{parameter}.scene.png")
	monfile = Pathname("#{parameter}.check.png")
	srand($opts[:seed].to_i)
	
	$tmpdir = Pathname("#{TMPDIR}/#{$myname}.#{parameter}")
	$termsize = $opts[:termsize] || 12.5 # Radius of terminal [mm]
	$batch_unit = $opts[:batch] || 100
	
	scn = MyScene.new(floorplan)
	if !logfile.exist? || $opts[:overwrite]
		$tmpdir.rmtree if $tmpdir.exist?
		$tmpdir.mkpath
		scn.put_terminals(termperrack, arrangement)
		scn.save_map(mapfile) if $opts[:save_map]
		scn.save_pbrt(pbrfile) if $opts[:save_pbrt]
		scn.render(scnfile) if $opts[:save_scene]
		if !$opts[:coldrun]
			if !$warning["Violation of territory"] || $opts[:allow_violation]
				STDERR.printf("Rendering starts at %s\n", st = Time.now) if $verbose
				scn.compute_lsr(parameter, monfile, $opts[:montage].to_i)
				STDERR.printf("Finished in %.1f seconds\n", Time.now - st) if $verbose
			end
			scn.save_log(logfile) if !$opts[:coldrun]
		end
		$tmpdir.rmdir rescue ""
	else
		scn.load_map(mapfile) if mapfile.exist?
		scn.load_log(logfile)
	end
	msg = ""
	$warning.each do |key, value|
		msg << sprintf("%s: %s; ", key, value)
		STDERR.printf("%s: %s: %s", parameter, key, value) if $verbose
	end
	STDOUT.printf("%s\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%s\n", parameter, scn.racks.length, scn.ni, scn.nj, scn.nk, $termsize, scn.lsr, scn.xmaxoffset, scn.ymaxoffset, scn.zmax, msg)
end

# ベクトル
class Vector
	def x; self[0]; end
	def y; self[1]; end
	def z; self[2]; end
	def x=(val); self[0] = val; end
	def y=(val); self[1] = val; end
	def z=(val); self[2] = val; end
	def cross_product(other)
		Vector.Raise ErrDimensionMismatch unless size == other.size && other.size == 3
		Vector[other[1] * self[2] - other[2] * self[1], other[2] * self[0] - other[0] * self[2], other[0] * self[1] - other[1] * self[0]]
	end
end

# 直線
class Line
	attr_accessor :pos, :dir
	def initialize(position, direction)
		@pos = Vector.elements(position) # 始点
		@dir = Vector.elements(direction) # 方向ベクトル
		@dir /= @dir.norm
	end
	def self.[](px, py, pz, dx, dy, dz)
		new([px.to_f, py.to_f, pz.to_f], [dx.to_f, dy.to_f, dz.to_f])
	end
	# 平行移動
	def move(vec)
		self.class.new(@pos + vec, @dir)
	end
	# 拡大縮小
	def scale(ratio)
		self.class.new(@pos * ratio, @dir * ratio)
	end
	# 向き
	def direction
		d = Vector[@dir.x, @dir.y, @dir.z]
		heading = Math.atan(d.y / d.x) / Math::PI * 180
		d = Vector[Math.sqrt(@dir.x**2 + @dir.y**2), 0, @dir.z]
		pitch = Math.atan(d.z / d.x) / Math::PI * 180
		[heading, pitch]
	end
	# 直線との交点
	def intersection(other)
		@pos + @dir * (((other.pos - @pos).cross_product(other.dir).inner_product(@dir.cross_product(other.dir))) / @dir.cross_product(other.dir).norm)
	end
	# 点 o を通るXY平面との交点
	def intersect_xy(o)
		@pos + @dir * ((o.z - @pos.z) / @dir.z)
	end
	# 点 o を通るXZ平面との交点
	def intersect_xz(o)
		@pos + @dir * ((o.y - @pos.y) / @dir.y)
	end
	# 点 o を通るYZ平面との交点
	def intersect_yz(o)
		@pos + @dir * ((o.x - @pos.x) / @dir.x)
	end
end

# 球面
class Sphere
	attr_accessor :pos, :radius
	def initialize(position, radius)
		@pos = Vector.elements(position)
		@radius = radius
	end
	def self.[](x, y, z, r)
		new([x.to_f, y.to_f, z.to_f], r.to_f)
	end
	def x; @pos.x; end
	def y; @pos.y; end
	def z; @pos.z; end
	def r; @radius; end
	# 平行移動
	def move(vec)
		self.class.new(@pos + vec, @radius)
	end
	# 拡大縮小
	def scale(ratio)
		self.class.new(@pos * ratio, @radius * ratio)
	end
	# 点 o を通る接線 (X方向にずらす)
	# 	sign = -1: X座標が小さい方を返す  1: X座標が大きい方を返す
	def tangent_x(o, sign)
		raise "Sign must be -1 or 1" unless sign == -1 || sign == 1
		s = (o - @pos) / @radius  # 座標変換 (自身が原点中心の単位円となるように)
		syz = Math.sqrt(s.y**2 + s.z**2) # 回転 (X軸まわりに回して z=0 とする)
		s1 = Vector[s.x, syz, 0.0]
		t = tangent_points(s1).max_by{|v| sign * v.x } # X座標が大きい方
		t = Vector[t.x, t.y * s.y / syz, t.y * s.z / syz] # 回転を元に戻す
		t = t * @radius + @pos # 座標変換を元に戻す
		Line.new(o, t - o) # 直線 ot を返す
	end
	# 点 o を通る接線 (Y方向にずらす)
	# 	sign = -1: Y座標が小さい方を返す  1: Y座標が大きい方を返す
	def tangent_y(o, sign)
		raise "Sign must be -1 or 1" unless sign == -1 || sign == 1
		s = (o - @pos) / @radius # 座標変換 (自身が原点中心の単位円となるように)
		sxz = Math.sqrt(s.x**2 + s.z**2) # 回転 (Y軸まわりに回して z=0 とする)
		s1 = Vector[sxz, s.y, 0.0]
		t = tangent_points(s1).max_by{|v| sign * v.y } # Y座標が大きい方
		t = Vector[t.x * s.x / sxz, t.y, t.x * s.z / sxz] # 回転を元に戻す
		t = t * @radius + @pos # 座標変換を元に戻す
		Line.new(o, t - o) # 直線 ot を返す
	end
	# 点 o を通る接線 (Z方向にずらす)
	# 	sign = -1: Z座標が小さい方を返す  1: Z座標が大きい方を返す
	def tangent_z(o, sign)
		raise "Sign must be -1 or 1" unless sign == -1 || sign == 1
		s = (o - @pos) / @radius # 座標変換 (自身が原点中心の単位円となるように)
		sxy = Math.sqrt(s.x**2 + s.y**2) # 回転 (Z軸まわりに回して y=0 とする)
		s1 = Vector[sxy, s.z, 0.0] # Z軸をY軸に読み替える
		t = tangent_points(s1).max_by{|v| sign * v.y } # Y座標 (元のZ座標) が大きい方
		t = Vector[t.x * s.x / sxy, t.x * s.y / sxy, t.y] # 回転を元に戻す。Y軸をZ軸に読み替える
		t = t * @radius + @pos # 座標変換を元に戻す
		Line.new(o, t - o) # 直線 ot を返す
	end
	# XY平面上の点 s を通り原点中心の単位円に接する接線の接点 (2か所)
	def tangent_points(s)
		raise "s must be on the XY plane" unless s.z == 0
		b = 1.0 / s.norm
		c = Math.sqrt(1.0 - b**2)
		u = s / s.norm # s方向の単位ベクトル
		v = Vector[-u.y, u.x, 0] # uと直角な単位ベクトル
		t1 = b * u + c * v # 接点1
		t2 = b * u - c * v # 接点2
		[t1, t2]
	end
end

# 直方体
class Cuboid
	attr_accessor :pos, :size
	attr :xmin, :xmax, :ymin, :ymax, :zmin, :zmax
	def initialize(position, size)
		@pos = Vector.elements(position) # 中心点
		@size = Vector.elements(size) # サイズ
		@xmin  = @pos.x - @size.x / 2.0
		@xmax  = @pos.x + @size.x / 2.0
		@ymin  = @pos.y - @size.y / 2.0
		@ymax  = @pos.y + @size.y / 2.0
		@zmin  = @pos.z - @size.z / 2.0
		@zmax  = @pos.z + @size.z / 2.0
	end
	def self.[](x, y, z, depth, width, height)
		new([x.to_f, y.to_f, z.to_f], [depth.to_f, width.to_f, height.to_f])
	end
	def x; @pos.x; end
	def y; @pos.y; end
	def z; @pos.z; end
	def d; @size.x; end
	def w; @size.y; end
	def h; @size.z; end
	# 点 o が内部にあるか?
	def inside?(o)
		@xmin <= o.x && o.x <= @xmax && @ymin <= o.y && o.y <= @ymax && @zmin <= o.z && o.z <= @zmax
	end
end

# マシンルーム
class MyScene
	attr :racks, :poles, :terms, :ni, :nj, :nk, :ci, :cj, :xmaxoffset, :ymaxoffset, :zmax, :lsr
	# フロアを作ってラックを配置
	#   座標系はマシンルームの中央を原点とする。
	#   ラックの上空部分を「ラック」として定義する。
	def initialize(floorplan)
		@racks = Array.new # @racks[n] = ラック
		@rack_at = Array.new # @rack_at[i][j] = ラック
		@terms = Array.new # @terms[n] = 端末
		@term_at = Array.new # @term_at[i][j][k] = 端末
		@ni = 0 # 列方向のラック数
		@nj = 0 # 行方向のラック数
		@nk = 0 # 1ラック上の端末数 (現在の実装は 1 に固定)
		@ci = 0 # 基準列番号
		@cj = 0 # 基準行番号
		@xmaxoffset = 0.0 # X方向の最大偏差
		@ymaxoffset = 0.0 # Y方向の最大偏差
		@zmax = 0.0 # Z方向の最大位置
		@lsr = 0.0 # 見通し率
		@lospx = 9 # 見通し画像サイズ (奇数)
		@losfov = 0.01 # 見通し画角
		rx = RACK_DEPTH + AISLE_DEPTH
		ry = RACK_WIDTH
		rz = RACK_HEADROOM
		case floorplan
		when /(\d+)x(\d+)/
			nrack = $1.to_i * $2.to_i
			@ni = $1.to_i
			@nj = $2.to_i
		when /(\d+)p/
			nrack = $1.to_i
			sq = Math.sqrt(nrack * rx * ry)
			@ni = (sq / rx).ceil
			@nj = (nrack.to_f / @ni).ceil
		when /(\d+)d/
			nrack = $1.to_i
			sq = Math.sqrt(nrack * rx * ry)
			@ni = (sq / rx).floor
			@ni = 1 if @ni < 1
			@nj = (nrack.to_f / @ni).ceil
		when /(\d+)n/
			nrack = $1.to_i
			@ni = Math.sqrt(nrack).ceil
			@nj = (nrack.to_f / @ni).ceil
		else
			raise "Invalid floorplan '#{floorplan}'"
		end
		@ci = (@ni - 1) / 2
		@cj = (@nj - 1) / 2
		cx = (@ni - 1) / 2.0 * rx # マシンルームの中央
		cy = (@nj - 1) / 2.0 * ry # 
		z = rz / 2.0 - $termsize
		nrack.times do |n|
			i = n / @nj
			j = n % @nj
			x = i * rx - cx
			y = j * ry - cy
			rack = Cuboid[x, y, z, rx, ry, rz]
			@racks[n] = rack
			@rack_at[i] ||= Array.new
			@rack_at[i][j] = rack
		end
	end
	# FSO端末を配置
	def put_terminals(nterm_per_rack, arrangement)
		nterm = nterm_per_rack * @racks.count
		@nk = nterm_per_rack
		nterm.times do |n|
			i = n / @nk / @nj
			j = n / @nk % @nj
			k = n % @nk
			x = @rack_at[i][j].x # ラックの中央
			y = @rack_at[i][j].y # 
			z = 0
			r = $termsize
			o = FSO_POLE_SIZE
# 			puts "[#{n}]\t#{i}\t#{j}\t#{k}\t#{x}\t#{y}\t#{z}"
			term = Sphere[x, y, z, r]
			@terms[n] = term
			@term_at[i] ||= Array.new
			@term_at[i][j] ||= Array.new
			@term_at[i][j][k] = term
		end
		
		raise "Not implemented" unless @nk == 1 # とりあえず
		case arrangement
		
		when /^Fix-Fix\b/
			# 何もしない
		when /^Fix-RndX\b/
			arrange_rndx
		when /^Fix-ArcX\b/
			arrange_arcx_fory
		when /^Fix-ArcZ\b/
			arrange_arcz_fory
			
		when /^RndY-Fix\b/
			arrange_rndy
		when /^RndY-RndX\b/
			arrange_rndy
			arrange_rndx
		when /^RndY-ArcX\b/
			arrange_arcx_fory
			arrange_rndy
		when /^RndY-ArcZ\b/
			arrange_arcz_fory
			arrange_rndy
		
		when /^ArcY-Fix\b/
			arrange_arcy_forx
		when /^ArcY-RndX\b/
			arrange_arcy_forx
			arrange_rndx
		when /^ArcY-ArcX\b/
			arrange_arcy_arcx # 特別
		when /^ArcY-ArcZ\b/
			arrange_arcz_fory
			arrange_arcy_forx
		
		when /^ArcZ-Fix\b/
			arrange_arcz_forx
		when /^ArcZ-RndX\b/
			arrange_arcz_forx
			arrange_rndx
		when /^ArcZ-ArcX\b/
			arrange_arcz_forx
			arrange_arcx_fory
		when /^ArcZ-ArcZ\b/
			arrange_arcz_forx
			arrange_arcz_fory
		
		when /^RndXY\b/
			arrange_rndy
			arrange_rndx
		when /^RndXYZ\b/
			arrange_rndz
			arrange_rndy
			arrange_rndx
		when /^Hybrid(\d+)\b/
			arrange_hybrid($1.to_i)
		else
			raise "Arrangement '#{arrangement}' is not defined"
		end
		
		# ラック上空に収まってるか確認
		@nj.times do |j|
			@ni.times do |i|
				r = @rack_at[i][j]
				t = @term_at[i][j][0]
				off = t.pos - r.pos
				@xmaxoffset = off.x.abs if @xmaxoffset < off.x.abs
				@ymaxoffset = off.y.abs if @ymaxoffset < off.y.abs
				@zmax = t.pos.z if @zmax < t.pos.z
				next if !(t.pos.x - t.r < r.xmin || r.xmax < t.pos.x + t.r || t.pos.y - t.r < r.ymin || r.ymax < t.pos.y + t.r || r.zmax < t.pos.z + t.r)
				msg = "Violation of territory"
				$warning[msg] ||= Hash.new
				$warning[msg]["X"] = sprintf("%.1f", off.x) if t.pos.x - t.r < r.xmin || r.xmax < t.pos.x + t.r
				$warning[msg]["Y"] = sprintf("%.1f", off.y) if t.pos.y - t.r < r.ymin || r.ymax < t.pos.y + t.r
				$warning[msg]["Z"] = sprintf("%.1f", t.pos.z) if r.zmax < t.pos.z + t.r
			end
		end
	end
	# RndX
	def arrange_rndx
		puts "<<<<<<<< arrange_rndx >>>>>>>>" if $verbose
		each do |rack, term, xsign, ysign|
			move_term_randomly(rack, term, true, false, false)
		end
	end
	# RndY
	def arrange_rndy
		puts "<<<<<<<< arrange_rndy >>>>>>>>" if $verbose
		each do |rack, term, xsign, ysign|
			move_term_randomly(rack, term, false, true, false)
		end
	end
	# RndZ
	def arrange_rndz
		puts "<<<<<<<< arrange_rndz >>>>>>>>" if $verbose
		each do |rack, term, xsign, ysign|
			move_term_randomly(rack, term, false, false, true)
		end
	end
	# X方向に ArcY
	def arrange_arcy_forx
		puts "<<<<<<<< arrange_arcy_forx >>>>>>>>" if $verbose
		each do |rack, term, xsign, ysign|
			move_term_to_edge(rack, term, 0, ysign)
		end
		@nj.times do |j|
			unroll_x(j) do |i, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				line = tx1.tangent_y(tx2.pos, -ysign)
				term.pos = line.intersect_yz(term.pos)
			end
		end
	end
	# Y方向に ArcX
	def arrange_arcx_fory
		puts "<<<<<<<< arrange_arcx_fory >>>>>>>>" if $verbose
		each do |rack, term, xsign, ysign|
			move_term_to_edge(rack, term, xsign, 0)
		end
		@ni.times do |i|
			unroll_y(i) do |j, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				line = ty1.tangent_x(ty2.pos, -xsign)
				term.pos = line.intersect_xz(term.pos)
			end
		end
	end
	# X方向に ArcZ
	def arrange_arcz_forx
		puts "<<<<<<<< arrange_arcz_forx >>>>>>>>" if $verbose
		@nj.times do |j|
			unroll_x(j) do |i, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				line = tx1.tangent_z(tx2.pos, 1)
				term.pos = line.intersect_yz(term.pos)
			end
		end
	end
	# Y方向に ArcZ
	def arrange_arcz_fory
		puts "<<<<<<<< arrange_arcz_fory >>>>>>>>" if $verbose
		@ni.times do |i|
			unroll_y(i) do |j, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				line = ty1.tangent_z(ty2.pos, 1)
				term.pos = line.intersect_xz(term.pos)
			end
		end
	end
	# X方向にArcY・Y方向にArcX
	def arrange_arcy_arcx
		puts "<<<<<<<< arrange_arcy_arcx >>>>>>>>" if $verbose
		# 全部を端に寄せる
		each do |rack, term, xsign, ysign|
			move_term_to_edge(rack, term, xsign, ysign)
		end
		# X軸2本
		@cj.upto(@cj+1) do |j|
			unroll_x(j) do |i, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				line = tx1.tangent_y(tx2.pos, -ysign)
				term.pos = line.intersect_yz(term.pos)
			end
		end
		# Y軸2本
		@ci.upto(@ci+1) do |i|
			unroll_y(i) do |j, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				line = ty1.tangent_x(ty2.pos, -xsign)
				term.pos = line.intersect_xz(term.pos)
			end
		end
		# その他
		(@cj+2).upto(@nj-1) do |j|
			unroll_x(j) do |i, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				xline = tx1.tangent_y(tx2.pos, -ysign)
				yline = ty1.tangent_x(ty2.pos, -xsign)
				term.pos = yline.intersection(xline)
			end
		end
		(@cj-1).downto(0) do |j|
			unroll_x(j) do |i, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				xline = tx1.tangent_y(tx2.pos, -ysign)
				yline = ty1.tangent_x(ty2.pos, -xsign)
				term.pos = yline.intersection(xline)
			end
		end
	end
	# ハイブリッド (両端の各 n 列は ArcZ-ArcX、残りは RndY-RndX)
	def arrange_hybrid(n)
		puts "<<<<<<<< arrange_hybrid(#{n}) >>>>>>>>" if $verbose
		bi = [n-1, @ci].min
		di = [@ci+1, @ni-n].max
		# 全部を端に寄せる
		each do |rack, term, xsign, ysign|
			move_term_to_edge(rack, term, xsign, 0)
		end
		# X方向に ArcZ
		@nj.times do |j|
			unroll_x(j) do |i, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				if i < bi || di < i
					line = tx1.tangent_z(tx2.pos, 1)
					term.pos = line.intersect_yz(term.pos)
				end
			end
		end
		# Y方向に ArcX または RndXY
		@ni.times do |i|
			unroll_y(i) do |j, rack, term, tx1, tx2, ty1, ty2, xsign, ysign|
				if i <= bi || di <= i
					line = ty1.tangent_x(ty2.pos, -xsign)
					term.pos = line.intersect_xz(term.pos)
				else
					term.pos.x = rand(rack.xmin..rack.xmax)
					term.pos.y = rand(rack.ymin..rack.ymax)
				end
			end
		end
	end
	# すべてのラックと端末をイテレート
	def each
		@ni.times do |i|
			xsign = (i <= @ci) ? -1 : 1 
			@nj.times do |j|
				ysign = (j <= @cj) ? -1 : 1
				yield(@rack_at[i][j], @term_at[i][j][0], xsign, ysign)
			end
		end
	end
	# X方向に中央から両端へイテレート
	def unroll_x(j)
		ysign = (j <= @cj) ? -1 : 1
		j1 = j - ysign
		j2 = j - ysign * 2
		Array(@ci..@ni-1).each_cons(3) do |i2, i1, i|
			yield(i, @rack_at[i][j], @term_at[i][j][0], @term_at[i1][j][0], @term_at[i2][j][0], @term_at[i][j1][0], @term_at[i][j2][0], 1, ysign)
		end
		Array(0..@ci+1).reverse.each_cons(3) do |i2, i1, i|
			yield(i, @rack_at[i][j], @term_at[i][j][0], @term_at[i1][j][0], @term_at[i2][j][0], @term_at[i][j1][0], @term_at[i][j2][0], -1, ysign)
		end
	end
	# Y方向に中央から両端へイテレート
	def unroll_y(i)
		xsign = (i <= @ci) ? -1 : 1
		i1 = i - xsign
		i2 = i - xsign * 2
		Array(@cj..@nj-1).each_cons(3) do |j2, j1, j|
			yield(j, @rack_at[i][j], @term_at[i][j][0], @term_at[i1][j][0], @term_at[i2][j][0], @term_at[i][j1][0], @term_at[i][j2][0], xsign, 1)
		end
		Array(0..@cj+1).reverse.each_cons(3) do |j2, j1, j|
			yield(j, @rack_at[i][j], @term_at[i][j][0], @term_at[i1][j][0], @term_at[i2][j][0], @term_at[i][j1][0], @term_at[i][j2][0], xsign, -1)
		end
	end
	# 端末をラックの端に寄せる
	def move_term_to_edge(rack, term, xsign, ysign)
		term.pos.x = ((xsign < 0) ? rack.xmin + term.r : rack.xmax - term.r) if xsign != 0
		term.pos.y = ((ysign < 0) ? rack.ymin + term.r : rack.ymax - term.r) if ysign != 0
	end
	# 端末をランダムにずらす
	def move_term_randomly(rack, term, xflag, yflag, zflag)
		term.pos.x = rand((rack.xmin + term.r)..(rack.xmax - term.r)) if xflag
		term.pos.y = rand((rack.ymin + term.r)..(rack.ymax - term.r)) if yflag
		term.pos.z = rand((rack.zmin + term.r)..(rack.zmax - term.r)) if zflag
	end
	# 端末の座標をファイルから読み込む
	def load_map(filename)
		@terms = Array.new
		@term_at = Array.new # 読み込まない
		open(filename, "r") do |f|
			f.each_line do |line|
				x, y, z, r = line.split
				next unless x && y && z && r
				x = x.to_i
				y = y.to_i
				z = z.to_i
				r = r.to_i
				term = Sphere[x, y, z, r]
				@terms.push(term)
			end
		end
	end
	# 端末の座標をファイルに書き出す
	def save_map(filename)
		open(filename, "w") do |f|
			@terms.each do |term|
				f.printf("%f %f %f %.1f\n", term.x, term.y, term.z, term.r)
			end
		end
	end
	# ログファイルから読み込む
	def load_log(filename)
		open(filename, "r") do |f|
			f.each_line do |line|
				case line
				when /^nrack\s*=\s*([-\d.]+)/ then @racks = Array.new($1.to_i)
				when /^nterm\s*=\s*([-\d.]+)/ then @terms = Array.new($1.to_i)
				when /^ni\s*=\s*([-\d.]+)/ then @ni = $1.to_i
				when /^nj\s*=\s*([-\d.]+)/ then @nj = $1.to_i
				when /^nk\s*=\s*([-\d.]+)/ then @nk = $1.to_i
				when /^ci\s*=\s*([-\d.]+)/ then @ci = $1.to_i
				when /^cj\s*=\s*([-\d.]+)/ then @cj = $1.to_i
				when /^lsr\s*=\s*([-\d.]+)/ then @lsr = $1.to_f
				when /^xmaxoffset\s*=\s*([-\d.]+)/ then @xmaxoffset = $1.to_f
				when /^ymaxoffset\s*=\s*([-\d.]+)/ then @ymaxoffset = $1.to_f
				when /^zmax\s*=\s*([-\d.]+)/ then @xmax = $1.to_f
				when /^warning\[Violation of territory\]\s*=\s*(.+)$/ then $warning["Violation of territory"] = $1
				end
			end
		end
	end
	# ログファイルに書き出す
	def save_log(filename)
		open(filename, "w") do |f|
			f.printf("nrack = %d\n", @racks.length)
			f.printf("nterm = %d\n", @terms.length)
			f.printf("ni = %d\n", @ni)
			f.printf("nj = %d\n", @nj)
			f.printf("nk = %d\n", @nk)
			f.printf("ci = %d\n", @ci)
			f.printf("cj = %d\n", @cj)
			f.printf("lsr = %f\n", @lsr)
			f.printf("xmaxoffset = %d\n", @xmaxoffset)
			f.printf("ymaxoffset = %d\n", @ymaxoffset)
			f.printf("zmax = %d\n", @zmax)
			$warning.each do |key, value|
				f.printf("warning[%s] = %s\n", key, value)
			end
		end
	end
	# シーン全体をPBRTファイルに書き出す
	def save_pbrt(filename)
		tgafile = Pathname(filename).sub_ext(".tga")
		open(filename, "w") do |f|
			write_pbrt(f, tgafile, 2400, 800, 30)
		end
	end
	# シーン全体をレンダリング
	def render(filename)
		tgafile = Pathname("#{$tmpdir}/scene.tga")
		IO.popen("pbrt >/dev/null 2>/dev/null", "w") do |pipe|
			write_pbrt(pipe, tgafile, 2400, 800, 30)
		end
		spawn("convert #{tgafile} #{filename} && rm #{tgafile}")
	end
	# すべてのショットをレンダリングして見通し率を計算 (ファイル経由)
	# 	小さすぎて効率が悪いので $batch_unit 個ずつに分けて並列実行
	def compute_lsr(parameter, monfile, monmax)
		px = @lospx / 2 # 画像中心ピクセル位置
		montage = (@terms.length <= monmax) # モンタージュ作成フラグ
		tgawild = "#{$tmpdir}/*.tga"
		cmd1 = "pbrt --quiet --ncores 1 %s 2> /dev/null"
		cmd2 = "#{$mydir}/pixelcolor %d %d %s"
		itr = Array(0...@terms.length).combination(2).each_slice($batch_unit)
		results = Parallel.map(itr) do |pairs|
			pbrtfile = "#{$tmpdir}/pid#{$$}.pbrt" # 並列実行される子プロセスごとに作る
			tgafiles = ""
			open(pbrtfile, "w") do |io|
				pairs.each do |camera, target|
					tgafile = "#{$tmpdir}/#{camera}-#{target}.tga"
					write_pbrt(io, tgafile, @lospx, @lospx, @losfov, camera, target)
					tgafiles << tgafile << " "
				end
			end
			system(cmd1 % [pbrtfile])
			File.delete(pbrtfile)
# 			# ちなみにパイプ経由の場合。端末数×$batch_unitが?大きすぎると Broken pipe になる
# 			IO.popen(cmd1 % [""], "w") do |io|
# 				pairs.each do |camera, target|
# 					tgafile = "#{$tmpdir}/#{camera}-#{target}.tga"
# 					write_pbrt(io, tgafile, @lospx, @lospx, @losfov, camera, target)
# 					tgafiles << tgafile << " "
# 				end
# 			end
			count = 0
			total = 0
			IO.popen(cmd2 % [px, px, tgafiles], "r") do |io|
				io.each_line do |line|
					if /^([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})$/ =~ line
						count += 1 if !($1 == $2 && $2 == $3 && $3 == $1)
						total += 1
					end
				end
			end
			File.delete(*tgafiles.split(" ")) if !montage
			[count, total]
		end
		sumcount = results.map{|x| x[0] }.inject{|r, x| r + x }
		sumtotal = results.map{|x| x[1] }.inject{|r, x| r + x }
		npairs = @terms.length * (@terms.length - 1) / 2
		raise "Unmatch number of pairs" unless sumtotal == npairs
		@lsr = sumcount / sumtotal.to_f
		return if !montage
		cmd4 = "montage -geometry +1+1 #{tgawild} #{monfile}"
		cmd5 = "rm #{tgawild}"
		cmd6 = "rmdir #{$tmpdir}"
		spawn("#{cmd4} && #{cmd5} && #{cmd6}")
	end
	# シーンをPBRT形式でIOに書き出す
	# 	io = 開かれたIOオブジェクト
	# 	outfn = 画像ファイル名
	# 	xsize = 画像の横サイズ
	# 	ysize = 画像の縦サイズ
	# 	fov = カメラの画角
	# 	camera = カメラにする端末番号  nil:シーン全体を撮る
	# 	target = 被写体にする端末番号
	def write_pbrt(io, outfn, xsize, ysize, fov, camera = nil, target = nil)
		if !@pbrt_spheres
			# 原点から X, Y, Z の各方向へ ±1 の大きさの立方体に押し込む
			surface    = @racks.map{|rack| rack.zmin }.min
			xmin, xmax = @terms.map{|term| term.x }.minmax
			ymin, ymax = @terms.map{|term| term.y }.minmax
			zmin, zmax = @terms.map{|term| term.z }.minmax
			termsize   = @terms.map{|term| term.r }.max
			roomsize = [(xmax - xmin).abs, (ymax - ymin).abs, (zmax - zmin).abs].max
			trans = Vector[xmin + xmax, ymin + ymax, zmin + zmax] / -2.0
			ratio = 2.0 / (roomsize + termsize * 2)
			@pbrt_spheres = @terms.map{|term|
				sph = term.move(trans)
				sph.radius -= FSO_BEAM_SIZE # 光線の太さ分だけ余裕を持たせる
				sph.scale(ratio)
			}
			@pbrt_polesize = FSO_POLE_SIZE * ratio
			@pbrt_surface = (surface + trans.z) * ratio
		end
		header_desc = <<-EOS
Film "image" "integer xresolution" [%d] "integer yresolution" [%d] "string filename" "%s"
LookAt %f %f %f %f %f %f 0 0 1
Camera "perspective" "float fov" [%f]
WorldBegin
LightSource "infinite" "rgb L" [1 1 1] "integer nsamples" [4]
Material "matte" "rgb Kd" [0 0 0]
		EOS
		target_desc = <<-EOS
TransformBegin
Translate %f %f %f
AttributeBegin
Material "plastic" "rgb Kd" [1 0 0]
Shape "sphere" "float radius" [%f]
AttributeEnd
Shape "cylinder" "float radius" [%f] "float zmin" [%f] "float zmax" [0]
TransformEnd
		EOS
		others_desc = <<-EOS
TransformBegin
Translate %f %f %f
Shape "sphere" "float radius" [%f]
Shape "cylinder" "float radius" [%f] "float zmin" [%f] "float zmax" [0]
TransformEnd
		EOS
		footer_desc = <<-EOS
WorldEnd
		EOS
		if camera && target # ひとつのショットを撮る場合
			cam = @pbrt_spheres[camera]
			trg = @pbrt_spheres[target]
		else # シーン全体を撮る場合
			cam = Vector[0.0, -2.0, 0.5]
			trg = Vector[0, 0, 0]
			others_desc = target_desc # 全端末を赤くする
		end
		io.printf(header_desc, xsize, ysize, outfn, cam.x, cam.y, cam.z, trg.x, trg.y, trg.z, fov)
		@pbrt_spheres.each do |sph|
			next if sph == cam
			io.printf((sph == trg) ? target_desc : others_desc, sph.x, sph.y, sph.z, sph.r, @pbrt_polesize, @pbrt_surface - sph.z)
		end
		io.print(footer_desc)
	end
end

main if $0 == __FILE__
