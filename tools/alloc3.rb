#!/usr/bin/ruby -Ku
# 
# 	NoCコア配置最適化 for SWoPP2013 & MCSoC2013
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

help_message = <<-EOM
Usage:
  #{$0} [options] input_file method parameters...
Options:
  -r	Reproduce the results from log file. Don't run solver
  -f	Overwrite existing log file
  -q	Be quiet
  --layout TYPE
  	mesh => Square-mesh layout (e.g. 4 x 4 nodes)
  	room => Square-room layout (e.g. 2 x 8 nodes) (default)
  --overhead OUTER,INNER
  	Inter-rack and intra-rack additional cable length [cm]
  --edge EDGELIST
  	Create egdelist with cable length
  	Give a nodewise (pre-clustering) edgelist filename
  --hist	Output histogram of cable length instead of TSV
  --coord	Output coordinates of racks instead of TSV
Input file:
  *.edges	Weighted edgelist
  *.dat	QAPLIB format (layout included)
Method and parameters:
  grasp   dense|sparse      => Greedy Randomized Adaptive Search by Resende
  fant    #iteration #trial => Fast Ant Colony by Taillard
  sa      #iteration #trial => Simulated Annealing by Taillard
  ts                        => Taboo Search by Taillard
  scop    #iteration [seed] => Taboo Search by SCOP
  zigzag                    => Zigzag by Fujiwara
  oneway                    => Oneway by Fujiwara
  base                      => Better one of Oneway/Zigzag
EOM

require "optparse"
require "time"

def main
	OPTS = Hash.new
	opt = OptionParser.new
	opt.on("-r") do |v|
		OPTS[:reproduce] = v
	end
	opt.on("-f") do |v|
		OPTS[:overwrite] = v
	end
	opt.on("-q") do |v|
		OPTS[:quiet] = v
	end
	opt.on("--layout TYPE") do |v|
		OPTS[:layout] = v
	end
	opt.on("--overhead OUTER,INNER") do |v|
		OPTS[:overhead] = v
	end
	opt.on("--edge EDGELIST") do |v|
		OPTS[:edge] = v
	end
	opt.on("--hist") do |v|
		OPTS[:output_histogram] = v
	end
	opt.on("--coord") do |v|
		OPTS[:output_coordinates] = v
	end
	opt.parse!(ARGV)
	if (OPTS[:reproduce] && OPTS[:overwrite])
		STDERR.puts "Options -r and -f cannot be used at the same time."
		exit(1)
	end
	unless (ARGV[0])
		puts help_message
		exit(1)
	end
	topology = File.basename(ARGV[0], ".*")
	method = ARGV[1]
	clufn = "#{topology}.log" # クラスタリングのログ
	logfn = "#{topology}.#{method}.log"
	outfn = "#{topology}.#{method}.edges"
	codfn = "#{topology}.#{method}.coord"
	if (OPTS[:edge] && !File.size?(OPTS[:edge]))
		STDERR.puts (OPTS[:edge] == true) ? "Pre-clustering edgelist file must be specified to create the weighted edgelist." : "'#{OPTS[:edge]}' is required to create the weighted edgelist."
		exit(1)
	end
	if ((OPTS[:overhead] || OPTS[:edge]) && !File.size?(clufn))
		STDERR.puts "'#{clufn}' is required to calculate the overhead or to create the edgelist."
		exit(1)
	end
	
	#===========================================================
	#   入力
	#===========================================================
	case ARGV[0]
	when /\.edges$/
		# エッジリストを読み込む
		flow = MyMatrix.load_edgelist(ARGV[0])
		case OPTS[:layout]
		when "mesh"
			layout = SquareMeshLayout.new(flow.row_size)
		when "room"
			layout = SquareRoomLayout.new(flow.row_size)
		else
			puts help_message
			exit(1)
		end
		dist = layout.manhattan_matrix
		q = QAP.new(dist, flow)
	when /\.dat$/
		# QAPLIB 形式を読み込む
		s = IO.readlines(ARGV[0], "") # paragraph mode
		dist = MyMatrix.new.scan(s[1])
		flow = MyMatrix.new.scan(s[2])
		q = QAP.new(dist, flow)
	else
		puts help_message
		exit(1)
	end
	
	case ARGV[1]
	when "grasp"
		q.solver = GraspQapSolver.new(ARGV[2])
	when "fant"
		q.solver = FantQapSolver.new(ARGV[2].to_i, ARGV[3].to_i)
	when "sa"
		q.solver = SaQapSolver.new(ARGV[2].to_i, ARGV[3].to_i)
	when "ts"
		q.solver = TsQapSolver.new()
	when "scop"
		q.solver = ScopQapSolver.new(ARGV[2].to_i, ARGV[3].to_i)
	when "zigzag"
		q.solver = BaselineQapSolver.new(layout, "zigzag")
	when "oneway"
		q.solver = BaselineQapSolver.new(layout, "oneway")
	when "base"
		q.solver = BaselineQapSolver.new(layout, nil)
	else
		puts help_message
		exit(1)
	end
	
	if (OPTS[:overhead] || OPTS[:edge])
		# クラスタリングログを読み込む
		s = IO.read(clufn)
		n_edge = s[/^#edge *= *(\d+)/, 1].to_i # 全リンク数
		n_tube = s[/^#tube *= *(\d+)/, 1].to_i # ラック外チューブ数
		n_link = s[/^#link *= *(\d+)/, 1].to_i # ラック外リンク数
		n_node = s[/^#node *= *(\d+)/, 1].to_i # 全ノード数
		membership = s[/^membership *= *(.+)$/, 1] # メンバーシップベクトル
		if (membership)
			membership = membership.scan(/\d+/).collect{|x| x.to_i }
		elsif (OPTS[:edge])
			STDERR.puts "Membership vector must exist in '#{clufn}' to create the weighted edgelist."
			exit(1)
		end
	end
	
	#===========================================================
	#   求解
	#===========================================================
	if ((!File.size?(logfn) && !OPTS[:reproduce]) || OPTS[:overwrite])
		open(logfn, "w") do |logf|
			logf.sync = true
			q.solve(logf)
		end
	elsif (File.size?(logfn))
# 		STDERR.puts "Reproducing the result from '#{logfn}'" unless OPTS[:quiet]
		open(logfn, "r") do |logf|
			q.reproduce(logf)
		end
	else
		STDERR.puts "Log file '#{logfn}' is required to reproduce the result."
		exit(1)
	end
	# オーバーヘッド計算
	if (OPTS[:overhead])
		outerhead = OPTS[:overhead].scan(/\d+/)[0].to_i
		innerhead = OPTS[:overhead].scan(/\d+/)[1].to_i
	else
		outerhead = 0
		innerhead = 0
	end
	
	# スケール計算
	scale = Math.sqrt(n_node / q.size);
	if scale.round != scale
		STDERR.puts "Cluster size must be a squared integer."
		exit(1)
	end
	scale = scale.round
	
	# ヒストグラム計算
	if (layout)
		tile = layout.tile
		slots = layout.max_length / tile
	else
		tile = 1
		slots = 0
		dist.each do |i, j, w|
			slots = w if (slots < w)
		end
	end
	hist = Array.new(slots, 0)
	longest = 0
	flow.each do |i, j, w|
		next if i > j # undirected
		len = dist[q.position[i], q.position[j]]
		hist[len / tile] += w
		longest = len if longest < len
	end
	legend = Array.new()
	slots.times do |t|
		legend[t] = t * tile
	end
	cdf = Array.new()
	hist.inject(0) do |r, w|
		cdf.push(r + w)
		r + w
	end
	
	#===========================================================
	#   出力
	#===========================================================
	if (OPTS[:output_histogram])
		# ケーブル長のヒストグラムを出力
		puts ["", "", legend].join("\t")
		puts [topology, method, hist].join("\t")
	else
		# サマリーを出力
		if (layout)
			# レイアウトモード
# 			v1 = q.evaluate_unweighted(q.solution)
# 			v2 = q.evaluate(q.solution)
# 			v3 = v1 + n_tube * outerhead
# 			v4 = v2 + n_link * outerhead + (n_edge - n_link) * innerhead
			head = n_link * outerhead + (n_edge - n_link) * innerhead
			objs = q.objectives.collect{|x| x * scale + head }
			v1 = objs.join(";")
			v2 = objs.stddev
			v3 = q.evaluate(q.solution).to_f * scale + head
			v4 = n_edge
			v5 = longest * scale + outerhead # 最長ケーブルはラック外と仮定
			obj = q.objective * scale
			sol = q.solution.join(";")
		else
			# 検証モード (QAPLIB 掲載書式に合わせる)
			v1 = q.evaluate_unweighted(q.solution) * 2
			v2 = q.evaluate(q.solution) * 2
			v3 = 0
			v4 = 0
			v5 = 0
			obj = q.objective * 2
			sol = "(" + q.solution.collect{|n| n + 1 }.join(",") + ")"
		end
		timestamp = Time.now.strftime("%Y-%m-%d %H:%M:%S")
		puts [topology, method, q.size, q.iterations, v1, v2, v3, v4, v5, obj, q.elapsed, timestamp, "", cdf].join("\t")
	end
	if (OPTS[:output_coordinates])
		# 座標リストを出力
		if (layout)
			open(codfn, "w") do |f|
				q.solution.each_with_index do |rack, k|
					f.puts [rack, layout.position(k)].join(" ")
				end
			end
		end
	end
	if (OPTS[:edge])
		# ケーブル長を重みとするノード単位のエッジリストを生成
		g = MyMatrix.load_edgelist(OPTS[:edge], false) # directed
		open(outfn, "w") do |f|
			g.each do |i, j, w|
				x = dist[q.position[membership[i]], q.position[membership[j]]]
				x += (x == 0) ? innerhead : outerhead
				f.printf("%d %d %d\n", i, j, x)
			end
		end
	end
end


# http://d.hatena.ne.jp/sesejun/20070502/p1
class Array
  def sum_with_number
    s = 0.0
    n = 0
    self.each do |v|
      next if v.nil?
      s += v.to_f
      n += 1
    end
    [s, n]
  end
  def sum
    s, n = self.sum_with_number
    s
  end
  def avg
    s, n = self.sum_with_number
    s / n
  end
  def var
    c = 0
    mean = self[c].to_f
    sum = 0.0
    n = 1
    (c+1).upto(self.size-1) do |i|
      next if self[i].nil?
      sweep = n.to_f / (n + 1.0)
      delta = self[i].to_f - mean
      sum += delta * delta * sweep
      mean += delta / (n + 1.0)
      n += 1
    end
    sum / n.to_f
  end
  def stddev
    Math.sqrt(self.var)
  end
end

# 行列
# 行番号・列番号は0オリジン
class MyMatrix
	def initialize()
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # 行データのデフォルト値
		}
		@row_size = 0
		@col_size = 0
	end
	attr_reader :row_size, :col_size
	def [](i, j)
		@data[i][j]
	end
	def []=(i, j, val)
		@data[i][j] = val
		@row_size = i + 1 if (row_size < i + 1)
		@col_size = j + 1 if (col_size < j + 1)
	end
	def empty?()
		@data.empty?
	end
	def each()
		@data.each do |i, r|
			r.each do |j, val|
				yield(i, j, val)
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
	def to_s()
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
	def self.load_edgelist(filename, undirected = true)
		a = self.new
		IO.foreach(filename) do |line|
			i, j, val = line.split # split by whitespaces
			next unless (i && j)
			i = i.to_i
			j = j.to_i
			val = (val) ? val.to_i : 1
			a[i, j] = val
			a[j, i] = val if (undirected)
		end
		a
	end
end

# ラック配置
class RackLayout
	def initialize(n)
		@nrack = n.to_i # ラック数
		@width = 1 # ラック幅 (cm)
		@depth = 1 # ラック奥行 (cm)
		@aisle = 0 # 通路幅 (cm)
		@tile  = 1 # 最大公約数
		# ラックを横一列に並べる
		if (@nrack > 0)
			@ny = 1 # 奥行方向の設置数
			@nx = @nrack # 幅方向の設置数
		else
			@ny = 0
			@nx = 0
		end
	end
	attr_reader :nrack, :nx, :ny, :width, :depth, :aisle, :tile
	# 部屋の幅
	def room_width()
		@nx * @width
	end
	# 部屋の奥行
	def room_depth()
		@ny * (@depth + @aisle)
	end
	# 部屋の面積
	def room_area()
		room_width * room_depth
	end
	# 最長距離
	def max_length()
		room_width + room_depth
	end
	# 列ごとにイテレート
	def each_row()
		(0...@nrack).each_slice(@nx) do |row|
			yield(row)
		end
	end
	def each_row_with_index()
		i = 0
		(0...@nrack).each_slice(@nx) do |row|
			yield(row, i)
			i += 1
		end
	end
	# ラックの座標 [x, y]
	def position(i)
		[(i % @nx) * @width, (i / @nx) * (@depth + @aisle)]
	end
	# マンハッタン距離
	def manhattan(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * (@depth + @aisle)
		x.abs + y.abs
	end
	# マンハッタン距離行列
	def manhattan_matrix()
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = manhattan(i, j)
			end
		end
		a
	end
end

class SquareRoomLayout < RackLayout
	def initialize(n)
		super(n)
		# なるべく正方形に近い部屋とする
		if (@nrack > 0)
			ra = Math.sqrt(@nrack * @width * (@depth + @aisle))
			@ny = (ra / (@depth + @aisle)).ceil # 奥行方向の設置数
			@nx = (@nrack.to_f / @ny).ceil       # 幅方向の設置数
			if (@ny > 1)
				ny2 = @ny - 1
				nx2 = (@nrack.to_f / ny2).ceil
				if (nx2 * ny2 < @nx * @ny)
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
		if (@nrack > 0)
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
		raise "Distance matrix is not square (#{@dist.row_size} rows, #{@dist.col_size} cols)" unless (@dist.row_size == @dist.col_size)
		raise "Flow matrix is not square (#{@flow.row_size} rows, #{@flow.col_size} cols)" unless (@flow.row_size == @flow.col_size)
		raise "Unmatch matrix size (dist: #{@dist.row_size}x#{@dist.col_size}, flow: #{@flow.row_size}x#{@flow.col_size})" unless (@dist.row_size == @flow.row_size)
		@size = @dist.row_size # 問題サイズ
		@solver = nil # ソルバー
		@solution = Array.new(@size) # 解 @solution[位置] = 施設
		@position = Array.new(@size) # 解 @position[施設] = 位置
		@objective = nil # 目的関数値
		@objectives = Array.new() # 目的関数値リスト
		@iterations = nil # 反復回数
		@inverse = false # 距離行列と重み行列を入れ替えて評価するフラグ
		@elapsed = 0 # 計算時間
	end
	attr_reader :size, :dist, :flow
	attr_accessor :solver, :solution, :position, :objective, :objectives, :iterations, :inverse, :elapsed
	# 求解
	def solve(logf)
		raise "No solver set" unless (@solver.is_a?(QapSolver))
		@solver.run(self, logf)
	end
	# ログファイルから再現
	def reproduce(logf)
		raise "No solver set" unless (@solver.is_a?(QapSolver))
		@solver.reproduce(self, logf)
	end
	# QAPLIB形式の文字列
	def to_s()
		"#{@size}\n\n#{@dist}\n\n#{@flow}\n"
	end
	# 評価関数値を計算
	def evaluate(sol)
		raise "Unmatch size (matrix=#{@size}x#{@size}, solution=#{sol.size})" unless (@size == sol.size)
		raise "Cannot invert the solution. Maybe 1-origin?" unless (sol.sort.uniq == (0...@size).to_a)
		sum = 0
		if (@inverse)
			@flow.each do |i, j, w|
				sum += w * @dist[sol[i], sol[j]]
			end
		else
			@dist.each do |i, j, w|
				sum += w * @flow[sol[i], sol[j]]
			end
		end
		sum / 2
	end
	# 評価関数値を重みなしで計算
	def evaluate_unweighted(sol)
		raise "Unmatch size (matrix=#{@size}x#{@size}, solution=#{sol.size})" unless (@size == sol.size)
		raise "Cannot invert the solution. Maybe 1-origin?" unless (sol.sort.uniq == (0...@size).to_a)
		sum = 0
		if (@inverse)
			@flow.each do |i, j, w|
				sum += w if (@dist[sol[i], sol[j]] > 0)
			end
		else
			@dist.each do |i, j, w|
				sum += w if (@flow[sol[i], sol[j]] > 0)
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
		if (q.size > 0)
			logf.puts(@cmd)
			IO.popen(@cmd, "r+") do |pipe|
				write_input(q, pipe)
				pipe.close_write
				read_output(q, pipe, logf)
			end
			q.solution.each_with_index do |x, i|
				q.position[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.position = []
		end
		finish_time = Time.now
		logf.puts finish_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		q.elapsed = finish_time - start_time
	end
	def reproduce(q, logf)
		read_output(q, logf, nil)
		q.solution.each_with_index do |x, i|
			q.position[x] = i
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
			if (!q.iterations || q.iterations < m[0].to_i)
				# 反復回数の最大値
				q.iterations = m[0].to_i
			end
			if (!q.objective || q.objective > m[1].to_i / 2)
				# 目的関数の最小値と、そのときの解
				q.objective = m[1].to_i / 2
				q.solution = m[2].scan(/\d+/).collect{|n| n.to_i }
			end
			q.objectives.push(q.objective)
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

# Taboo Search
class TsQapSolver < TaillardQapSolver
	def initialize()
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
						if (k != l)
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
					q.iterations = m[1].to_i if (m[0] == "iteration")
				end
			when /^\[best solution\]/
				s.scan(/^x\[(\d+)\]:\s*(\d+)$/).each do |m|
					q.solution[m[0].to_i] = m[1].to_i
				end
			when /^\[Violated constraints\]/
				s.scan(/^(\w+):\s*(\d+)$/).each do |m|
					q.objective = m[1].to_i if (m[0] == "objective")
				end
			end
		end
		q.inverse = true
	end
end

# Baseline
class BaselineQapSolver < QapSolver
	def initialize(layout, method = nil)
		raise "No layout given" unless (layout.is_a?(RackLayout))
		@lay = layout
		@method = method
	end
	def run(q, logf)
		if (q.size > 0)
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
				q.position[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.position = []
		end
	end
	def sol_oneway()
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
	def sol_zigzag()
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			row = row.reverse if (i.odd?)
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
			f.puts(line) if (f)
			str << line << "\n"
		end
		str
	end
end

# 本体
main if ($0 == __FILE__)
