#!/usr/bin/ruby -Ku
# 
# 	ラック配置最適化 (lay.rb の後に使う)
# 	Optimize rack layout by length/power/energy. Use after lay.rb
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

OVERHEAD_OUTER = 400 # Inter-rack cabling overhead [cm]
OVERHEAD_INNER = 200 # Intra-rack cabling overhead [cm]
RACK_WIDTH     =  60 # Rack width [cm]
RACK_DEPTH     = 210 # Rack depth including aisle [cm]

# Power consumption per switch port
def power_of(len)
	if len <= 700 # [cm]
		6600 # [mW]
	else
		12400 # [mW]
	end
end
# Energy consumption per packet
def energy_of(len)
	if len <= 700 # [cm]
		2 # [pJ]
	else
		60 # [pJ]
	end
end
# Cable length
def cable_of(dist)
	(dist == 0) ? OVERHEAD_INNER : OVERHEAD_OUTER + dist
end

$help_message = <<-EOM
Usage:
  #{$0} [Options] Input_file Objective Method

Options:
  -r	reproduce the results from log file; don't run solver
  -f	overwrite existing log file
  -q	be quiet
  --traffic FILENAME
    	per-node (non-clustered) traffic filename
    	required to calculate energy consumption
  --edge FILENAME
    	per-node (non-clustered) edgelist filename
    	required to create a per-node egdelist with cable length
  --euclidean
    	use diagonal cabling in place of rectilinear (Manhattan) cabling
  --hist
    	output histogram of cable length in addition to a summary

Input file:
  *.edges	per-rack (clustered) edgelist filename

Objective:
  ca => cable length [cm]
  po => power consumption [mW]
  en => energy consumption [pJ] considering traffic

Method:
  ts   => taboo search by Taillard
  base => better one of oneway & zigzag
EOM

require "optparse"
require "time"

def main
	$opts = Hash.new
	opt = OptionParser.new
	opt.on("-r") do |v|
		$opts[:reproduce] = v
	end
	opt.on("-f") do |v|
		$opts[:overwrite] = v
	end
	opt.on("-q") do |v|
		$opts[:quiet] = v
	end
	opt.on("--traffic FILENAME") do |v|
		$opts[:traffic] = v
	end
	opt.on("--edge FILENAME") do |v|
		$opts[:nodelist] = v
	end
	opt.on("--layout TYPE") do |v|
		$opts[:layout] = v
	end
	opt.on("--euclidean") do |v|
		$opts[:euclidean] = v
	end
	opt.on("--hist") do |v|
		$opts[:output_histogram] = v
	end
	opt.parse!(ARGV)
	unless ARGV[0] && ARGV[1] && ARGV[2]
		puts $help_message
		exit(1)
	end
	if $opts[:reproduce] && $opts[:overwrite]
		STDERR.puts "Options -r and -f cannot be used at the same time."
		exit(1)
	end
	topology = File.basename(ARGV[0], ".*")
	parameter = topology + "." + ARGV[1..2].join("-")
	layfn = topology.split(/\+/)[0] + ".coord"
	clufn = "#{topology}.log"
	logfn = "#{parameter}.log"
	outfn = "#{parameter}.edges"
	mapfn = "#{parameter}.coord"
	if !File.size?(clufn)
		STDERR.puts "'#{clufn}' must exist in the same directory."
		exit(1)
	end
	if !File.size?(layfn)
		STDERR.puts "'#{layfn}' must exist in the same directory."
		exit(1)
	end
	if $opts[:traffic] && !File.size?($opts[:traffic])
		STDERR.puts "'#{$opts[:traffic]}' is not available."
		exit(1)
	end
	if $opts[:nodelist] && !File.size?($opts[:nodelist])
		STDERR.puts "'#{$opts[:nodelist]}' is not available."
		exit(1)
	end
	
	#===========================================================
	#   Input
	#===========================================================
	# Read clusering log file
	s = IO.read(clufn)
	n_edge = s[/^#edge *= *(\d+)/, 1].to_i # Number of node-to-node connections
	n_tube = s[/^#tube *= *(\d+)/, 1].to_i # Number of rack-to-rack connections
	n_link = s[/^#link *= *(\d+)/, 1].to_i # Number of node-to-node connections outside racks
	membership = s[/^membership *= *(.+)$/, 1] # Membership vector
	if membership
		membership = membership.scan(/\d+/).collect{|x| x.to_i }
	else
		STDERR.puts "Membership vector must exist in '#{clufn}'."
		exit(1)
	end
	# Prepare connection matrix and distance matrix
	conn = MyMatrix.new.load_edgelist(ARGV[0])
	layout = RackLayout.new(conn.row_size).load_coordinates(layfn)
	if $opts[:euclidean]
		dist = layout.l2norm_matrix
	else
		dist = layout.l1norm_matrix
	end
	# Prepare traffic matrix
	traffic_inner = 0
	traf = MyMatrix.new
	if $opts[:traffic]
		MyMatrix.new.load_edgelist($opts[:traffic]).each do |v1, v2, w|
			if membership[v1] == membership[v2]
				traffic_inner += w
			else
				traf[membership[v1], membership[v2]] += w
			end
		end
	elsif ARGV[1] == "en"
		STDERR.puts "Traffic file must be specified as '--traffic FILENAME' to calculate energy consumption."
		exit(1)
	end
	# Prepare map
	case ARGV[1]
	when "ca"
		q = QAP.new(dist, conn)
	when "po", "el"
		q = QAP.new(dist.map{|val| power_of(cable_of(val)) }, conn)
	when "en"
		q = QAP.new(dist.map{|val| energy_of(cable_of(val)) }, traf)
	else
		puts $help_message
		exit(1)
	end
	# Prepare solver
	case ARGV[2]
	when "sa"
		q.solver = SaQapSolver.new(100000000, 10)
	when "ts"
		q.solver = RoTsQapSolver.new
	when "base"
		q.solver = BaselineQapSolver.new(layout, nil)
	else
		puts $help_message
		exit(1)
	end
	
	#===========================================================
	#   Solve
	#===========================================================
	if (!File.size?(logfn) && !$opts[:reproduce]) || $opts[:overwrite]
		open(logfn, "w") do |logf|
			logf.sync = true
			q.solve(logf)
		end
	elsif File.size?(logfn)
# 		STDERR.puts "Reproducing the result from '#{logfn}'" unless $opts[:quiet]
		open(logfn, "r") do |logf|
			q.reproduce(logf)
		end
	else
		STDERR.puts "Log file '#{logfn}' is required to reproduce the result."
		exit(1)
	end
	
	#===========================================================
	#   Output
	#===========================================================
	# Display summary
	v1 = q.evaluate(q.solution)
	v2 = (n_edge - n_link) * cable_of(0) # total cable length
	v3 = (n_edge - n_link) * power_of(cable_of(0)) # total power consumption
	v4 = traffic_inner * energy_of(cable_of(0)) # total energy consumption
	conn.each do |i, j, w|
		next if i >= j # unidirectional inter-rack cables
		len = cable_of(dist[q.location[i], q.location[j]])
		v2 += len * w
		v3 += power_of(len) * w
		v4 += energy_of(len) * traf[i, j]
	end
	obj = q.objective
	sol = q.solution.join(",")
	timestamp = Time.now.strftime("%Y-%m-%d %H:%M:%S")
	STDOUT.puts [parameter, q.size, v1, v2, v3, v4].join("\t")
# 	STDOUT.puts [parameter, q.size, v1, v2, v3, v4, q.elapsed, timestamp].join("\t")
	# Display histogram
	if $opts[:output_histogram]
		tile = layout.tile
		slots = layout.max_length / tile
		hist = Array.new(slots, 0)
		conn.each do |i, j, w|
			len = cable_of(dist[q.location[i], q.location[j]])
			slot = len / tile
			hist[slot] += w
		end
		legend = Array.new
		slots.times do |slot|
			legend[slot] = slot * tile
		end
		puts ["", legend].join("\t")
		puts [parameter, hist].join("\t")
	end
	# Write nodewise edgelist and coordinates
	if $opts[:nodelist]
		g = MyMatrix.new.load_edgelist($opts[:nodelist], false) # directed (to avoid duplication)
		open(outfn, "w") do |f|
			g.each do |v1, v2, w|
				len = cable_of(dist[q.location[membership[v1]], q.location[membership[v2]]])
				f.printf("%d %d %d\n", v1, v2, len)
			end
		end
		open(mapfn, "w") do |f|
			membership.each_with_index do |rack, node|
				x, y = layout.coord(q.location[rack])
				f.printf("%d %d %d %d\n", node, rack, x, y)
			end
		end
	end
end

# 行列
# 行番号・列番号は0オリジン
class MyMatrix
	include Enumerable
	def initialize
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # 行データのデフォルト値
		}
		@row_size = 0
		@col_size = 0
	end
	attr_reader :row_size, :col_size
	def initialize_copy(org)
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		org.each do |i, j, val|
			@data[i][j] = val
		end
	end
	def [](i, j)
		@data[i][j]
	end
	def []=(i, j, val)
		raise TypeError unless i.is_a?(Integer) && j.is_a?(Integer) && val.is_a?(Numeric)
		@data[i][j] = val
		@row_size = i + 1 if row_size < i + 1
		@col_size = j + 1 if col_size < j + 1
		self
	end
	# スカラー和
	def +(other)
		raise TypeError unless other.is_a?(Numeric)
		a = self.class.new
		@row_size.times do |i|
			@col_size.times do |j|
				a[i, j] = @data[i][j] + other
			end
		end
		a
	end
	# スカラー積
	def *(other)
		raise TypeError unless other.is_a?(Numeric)
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = val * other
			end
		end
		a
	end
	# 各要素にブロックを適用した結果を新しい行列で返す
	def map
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = yield(val)
			end
		end
		a
	end
	def empty?
		@data.empty?
	end
	def each
		@data.each do |i, r|
			r.each do |j, val|
				yield(i, j, val)
			end
		end
	end
	def each_index
		@row_size.times do |i|
			@col_size.times do |j|
				yield(i, j)
			end
		end
	end
	def each_index_upper(strict = false)
		raise "Square matrix expected" unless @row_size == @col_size
		d = (strict) ? 1 : 0
		(0...(@row_size - d)).each do |i|
			((i + d)...@col_size).each do |j|
				yield(i, j)
			end
		end
	end
	def along_row(i)
		@data[i].each do |j, val|
			yield(j, val)
		end
	end
	def along_col(j)
		@data.each do |i, r|
			yield(i, r[j])
		end
	end
	def to_s
		a = Array.new
		@row_size.times do |i|
			b = Array.new
			@col_size.times do |j|
				b.push(@data[i][j].to_s)
			end
			a.push(b.join(" "))
		end
		a.join("\n")
	end
	# スペース区切り文字列を読む
	def scan(str)
		str.each_with_index do |line, i|
			line.split.each_with_index do |val, j|
				self[i.to_i, j.to_i] = val.to_i
			end
		end
		self
	end
	# エッジリストを隣接行列として読み込む
	def load_edgelist(filename, undirected = true)
		IO.foreach(filename) do |line|
			i, j, val = line.split # split by whitespaces
			next unless i && j
			i = i.to_i
			j = j.to_i
			val = (val) ? val.to_i : 1
			self[i, j] = val
			self[j, i] = val if undirected
		end
		self
	end
end

# ラック配置
class RackLayout
	def initialize(n)
		@nrack = n.to_i # ラック数
		@width = RACK_WIDTH # ラック幅 (cm)
		@depth = RACK_DEPTH # ラック奥行＋通路幅 (cm)
		@tile  = @width.gcd(@depth) # 最大公約数
		if @nrack > 0
			@ny = 1 # 奥行方向の設置数
			@nx = @nrack # 幅方向の設置数
		else
			@ny = 0
			@nx = 0
		end
	end
	attr_reader :nrack, :nx, :ny, :width, :depth, :aisle, :tile
	# 部屋の幅
	def room_width
		@nx * @width
	end
	# 部屋の奥行
	def room_depth
		@ny * @depth
	end
	# 部屋の面積
	def room_area
		room_width * room_depth
	end
	# 最長距離
	def max_length
		room_width + room_depth
	end
	# 列ごとにイテレート
	def each_row
		(0...@nrack).each_slice(@nx) do |row|
			yield(row)
		end
	end
	def each_row_with_index
		i = 0
		(0...@nrack).each_slice(@nx) do |row|
			yield(row, i)
			i += 1
		end
	end
	# ラックの座標 [x, y]
	def coord(i)
		[(i % @nx) * @width, (i / @nx) * @depth]
	end
	# マンハッタン距離
	def l1norm(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * @depth
		x.abs + y.abs
	end
	# ユークリッド距離
	def l2norm(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * @depth
		Math.sqrt(x * x + y * y).round
	end
	# マンハッタン距離行列
	def l1norm_matrix
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = l1norm(i, j)
			end
		end
		a
	end
	# ユークリッド距離行列
	def l2norm_matrix
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = l2norm(i, j)
			end
		end
		a
	end
	# 座標ファイルを読み込む
	def load_coordinates(filename)
		rmax = 0
		xmax = 0
		ymax = 0
		IO.foreach(filename) do |line|
			n, r, x, y = line.split # split by whitespaces
			next unless n && r && x && y
			n = n.to_i
			r = r.to_i
			x = x.to_i
			y = y.to_i
			raise "Rack width unmatch with coordinates" unless x % @width == 0
			raise "Rack depth unmatch with coordinates" unless y % @depth == 0
			rmax = r if rmax < r
			xmax = x if xmax < x
			ymax = y if ymax < y
		end
		raise "Number of racks unmatch in edgelist and in coordinates" unless rmax + 1 == @nrack
		@nx = xmax / @width + 1
		@ny = ymax / @depth + 1
		self
	end
end

class SquareRoomLayout < RackLayout
	def initialize(n)
		super(n)
		# なるべく正方形に近い部屋とする
		if @nrack > 0
			ra = Math.sqrt(@nrack * @width * @depth)
			@ny = (ra / @depth).ceil # 奥行方向の設置数
			@nx = (@nrack.to_f / @ny).ceil      # 幅方向の設置数
			if @ny > 1
				ny2 = @ny - 1
				nx2 = (@nrack.to_f / ny2).ceil
				if nx2 * ny2 < @nx * @ny
					@nx = nx2
					@ny = ny2
				end
			end
		end
	end
end

class SquareMeshLayout < RackLayout
	def initialize(n)
		super(n)
		# ラックを縦横同数ずつ並べる
		if @nrack > 0
			@ny = Math.sqrt(@nrack).ceil   # 奥行方向の設置数
			@nx = (@nrack.to_f / @ny).ceil # 幅方向の設置数
		end
	end
end

# 二次割当問題
class QAP
	def initialize(dist_matrix, flow_matrix)
		@dist = dist_matrix # 距離行列
		@flow = flow_matrix # 重み行列
		raise "Distance matrix is not square (#{@dist.row_size} rows, #{@dist.col_size} cols)" unless @dist.row_size == @dist.col_size
		raise "Flow matrix is not square (#{@flow.row_size} rows, #{@flow.col_size} cols)" unless @flow.row_size == @flow.col_size
		raise "Unmatch matrix size (dist: #{@dist.row_size}x#{@dist.col_size}, flow: #{@flow.row_size}x#{@flow.col_size})" unless @dist.row_size == @flow.row_size
		@size = @dist.row_size # 問題サイズ
		@solver = nil # ソルバー
		@solution = Array.new(@size) # 解 @solution[位置] = 施設
		@location = Array.new(@size) # 解 @location[施設] = 位置
		@objective = nil # 目的関数値
		@iterations = nil # 反復回数
		@inverse = false # 距離行列と重み行列を入れ替えて評価するフラグ
		@elapsed = 0 # 計算時間
	end
	attr_reader :size, :dist, :flow
	attr_accessor :solver, :solution, :location, :objective, :iterations, :inverse, :elapsed
	# 求解
	def solve(logf)
		raise "No solver set" unless @solver.is_a?(QapSolver)
		@solver.run(self, logf)
	end
	# ログファイルから再現
	def reproduce(logf)
		raise "No solver set" unless @solver.is_a?(QapSolver)
		@solver.reproduce(self, logf)
	end
	# QAPLIB形式の文字列
	def to_s
		"#{@size}\n\n#{@dist}\n\n#{@flow}\n"
	end
	# 評価関数値を計算
	def evaluate(sol)
		raise "Unmatch size (matrix=#{@size}x#{@size}, solution=#{sol.size})" unless @size == sol.size
		raise "Cannot invert the solution. Maybe 1-origin?" unless sol.sort.uniq == (0...@size).to_a
		sum = 0
		unless @inverse
			@dist.each do |i, j, w|
				sum += w * @flow[sol[i], sol[j]]
			end
		else
			@flow.each do |i, j, w|
				sum += w * @dist[sol[i], sol[j]]
			end
		end
		sum / 2
	end
	# 評価関数値を重みなしで計算
	def evaluate_unweighted(sol)
		raise "Unmatch size (matrix=#{@size}x#{@size}, solution=#{sol.size})" unless @size == sol.size
		raise "Cannot invert the solution. Maybe 1-origin?" unless sol.sort.uniq == (0...@size).to_a
		sum = 0
		unless @inverse
			@dist.each do |i, j, w|
				sum += w if @flow[sol[i], sol[j]] > 0
			end
		else
			@flow.each do |i, j, w|
 				sum += w if @dist[sol[i], sol[j]] > 0
			end
		end
		sum / 2
	end
end

# 二次割当ソルバー
class QapSolver
	def run(q, logf)
		start_time = Time.now
		logf.puts start_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		if q.size > 0
			logf.puts(@cmd)
			IO.popen(@cmd, "r+") do |pipe|
				write_input(q, pipe)
				pipe.close_write
				read_output(q, pipe, logf)
			end
			q.solution.each_with_index do |x, i|
				q.location[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.location = []
		end
		finish_time = Time.now
		logf.puts finish_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		q.elapsed = finish_time - start_time
	end
	def reproduce(q, logf)
		read_output(q, logf, nil)
		q.solution.each_with_index do |x, i|
			q.location[x] = i
		end
		logf.rewind
		ts = logf.read.scan(/^===== (\d+-\d+-\d+ \d+:\d+:\d+) =====$/)
		start_time = Time.parse(ts[0][0])
		finish_time = Time.parse(ts[-1][0])
		q.elapsed = finish_time - start_time
	end
end

# GRASP
class GraspQapSolver < QapSolver
	def initialize(mode)
		here = File.dirname(File.expand_path(__FILE__))
		case mode
		when "dense"
			@cmd = "#{here}/gqapd"
		when "sparse"
			@cmd = "#{here}/gqaps"
		else
			raise "Option must be 'dense' or 'sparse'"
		end
	end
	def write_input(q, f)
		f.puts(q)
	end
	def read_output(q, f, logf)
		output = f.tee(logf)
		q.iterations = output[/^ *grasp iterations *: *(\d+)/, 1].to_i
		q.objective = output[/^ *cost of best permutation found *: *(\d+)/, 1].to_i / 2
		q.solution = output[/^ *best permutation found((\s*:( *\d+)+)+)/, 1].scan(/\d+/).collect{|n| n.to_i - 1 }
	end
end

class TaillardQapSolver < QapSolver
	def write_input(q, f)
		f.puts(q)
	end
	def read_output(q, f, logf)
		f.tee(logf).scan(/^@(\d+)\s+(\d+)\s+\[([\d,\s]+)\]/).each do |m|
			if !q.iterations || q.iterations < m[0].to_i
				# 反復回数の最大値
				q.iterations = m[0].to_i
			end
			if !q.objective || q.objective > m[1].to_i / 2
				# 目的関数の最小値と、そのときの解
				q.objective = m[1].to_i / 2
				q.solution = m[2].scan(/\d+/).collect{|n| n.to_i }
			end
		end
	end
end

# Fast Ant Colony
class FantQapSolver < TaillardQapSolver
	def initialize(n_iteration, n_trial)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/fant_qap #{n_iteration} #{n_trial}"
	end
end

# Simulated Annealing
class SaQapSolver < TaillardQapSolver
	def initialize(n_iteration, n_trial)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/sa_qap #{n_iteration} #{n_trial}"
	end
end
class SaQbapSolver < TaillardQapSolver
	def initialize(n_iteration, n_trial)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/sa_qbap #{n_iteration} #{n_trial}"
	end
end

# Robust Taboo Search
class RoTsQapSolver < TaillardQapSolver
	def initialize
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/tabou_qap"
	end
end

# SCOP
class ScopQapSolver < QapSolver
	def initialize(iteration = 1000000, seed = 1)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/wcsp -iteration #{iteration} -seed #{seed}"
	end
	def write_input(q, f)
		vars = (0...q.size).collect{|i| "x[#{i}]" }
		vals = (0...q.size).collect{|i| "#{i}" }
		vars.each do |var|
			f.print("variable #{var} in {#{vals.join(',')}}\n")
		end
		f.print("constraint: weight=inf type=alldiff #{vars.join(' ')} ;\n")
		f.print("objective: weight=1 type=quadratic\n")
		(0...(q.size - 1)).each do |i|
			((i + 1)...q.size).each do |j|
				(0...q.size).each do |k|
					(0...q.size).each do |l|
						if k != l
							f.print("#{q.flow[i, j] * q.dist[k, l]}(#{vars[i]},#{vals[k]})(#{vars[j]},#{vals[l]})\n")
						end
					end
				end
			end
		end
		f.print("<= 0\n")
	end
	def read_output(q, f, logf)
		f.tee(logf).each_line("") do |s| # paragraph mode
			case s
			when /^# iteration =/
				s.scan(/^#\s*(\w+)\s*=\s*(\d+)\/(\d+)/).each do |m|
					q.iterations = m[1].to_i if m[0] == "iteration"
				end
			when /^\[best solution\]/
				s.scan(/^x\[(\d+)\]:\s*(\d+)$/).each do |m|
					q.solution[m[0].to_i] = m[1].to_i
				end
			when /^\[Violated constraints\]/
				s.scan(/^(\w+):\s*(\d+)$/).each do |m|
					q.objective = m[1].to_i if m[0] == "objective"
				end
			end
		end
		q.inverse = true
	end
end

# Baseline
class BaselineQapSolver < QapSolver
	def initialize(layout, method = nil)
		raise "No layout given" unless layout.is_a?(RackLayout)
		@lay = layout
		@method = method
	end
	def run(q, logf)
		if q.size > 0
			case @method
			when "oneway"
				q.solution = sol_oneway
			when "zigzag"
				q.solution = sol_zigzag
			else
				sols = [sol_oneway, sol_zigzag]
				q.solution = sols.min_by{|sol| q.evaluate(sol) }
			end
			q.iterations = 0
			q.objective = q.evaluate(q.solution)
			q.solution.each_with_index do |x, i|
				q.location[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.location = []
		end
	end
	def sol_oneway
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
	def sol_zigzag
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			row = row.reverse if i.odd?
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
end

class IO
	# ログファイルに書き出しつつ文字列として返す
	def tee(f)
		str = ""
		self.each_line do |line|
			line = line.chomp
			f.puts(line) if f
			str << line << "\n"
		end
		str
	end
end

# 本体
main if $0 == __FILE__
